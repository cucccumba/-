#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <stdlib.h>
#include <math.h>
#include <pwd.h>
#include <grp.h>

#define print(buf_for_print_rights, name)                               \
{                                                                       \
    if (is_a)                                                           \
    {                                                                   \
        if (is_i)                                                       \
            printf("%lu  ", e->d_ino);                                  \
        if (is_l || is_n)                                               \
            print_rights(buf_for_print_rights, name, is_n);             \
        printf("%s\n",e->d_name);                                       \
    }                                                                   \
    else                                                                \
    {                                                                   \
        if (e->d_name[0] != '.')                                        \
        {                                                               \
            if (is_i)                                                   \
                printf("%lu  ", e->d_ino);                              \
            if (is_l || is_n)                                           \
                print_rights(buf_for_print_rights, name, is_n);         \
            printf("%s\n", e->d_name);                                  \
        }                                                               \
    }                                                                   \
}

struct argument
{
    char *name;
    int type;  //! 0: file, 1: directory
};


void print_rights(char *buf, char *name, int is_n)
{
    struct stat statbuf = {};
    char *path_end = strchr(buf, '\0');
    if(name)
    {
        sprintf(path_end, "%s", name);
    }
    if (stat(buf, &statbuf) < 0)
        perror("ERROR in print_rights");
    *path_end = '\0';
    switch (statbuf.st_mode & S_IFMT)
    {
        case S_IFREG:
        {
            printf("-");
            break;
        }
        case S_IFDIR:
        {
            printf("d");
            break;
        }
        case S_IFBLK:
        {
            printf("b");
            break;
        }
        case S_IFCHR:
        {
            printf("c");
            break;
        }
        case S_IFIFO:
        {
            printf("F");
            break;
        }
        case S_IFLNK:
        {
            printf("l");
            break;
        }
        case S_IFSOCK:
        {
            printf("s");
            break;
        }
    }

    int i = 0;
    for (int j = 8; j >= 0; --j)
    {
        if ((statbuf.st_mode >> j) & 1)
        {
            if (i == 0)
                printf("r");
            if (i == 1)
                printf("w");
            if (i == 2)
                printf("x");
        }
        else
            printf("-");
        ++i;
        if (i == 3)
            i = 0;
    }
    printf(" ");

    printf("%ld ", statbuf.st_nlink);

    if (is_n)
        printf("%d %d ", statbuf.st_uid, statbuf.st_gid);
    else
    {
        char* name = getpwuid(statbuf.st_uid)->pw_name;
        printf("%s %s ", name, name);
    }

    printf("%ld ", statbuf.st_size);

}
void print_dir(DIR *d, int is_a, int is_i, int is_l, int is_n)
{
    assert(d);

    struct dirent *e = readdir(d);
    while (e != NULL)
    {
        print(e->d_name, NULL);
        e = readdir(d);
    }
}

void print_dir_rec(DIR *d, char* dir_name, char *buf, int is_a, int is_i, int is_l, int is_n)
{
    assert(d);
    char *dirs[1024] = {};
    int i = 0;
    struct dirent *e = readdir(d);
    printf("%s:\n", dir_name);
    char buf_for_print_rights[512] = {};
    memcpy(buf_for_print_rights, buf, 512);
    while (e != NULL)
    {
        if (e->d_type == DT_DIR && strcmp(e->d_name, ".") != 0 && strcmp(e->d_name, "..") != 0)
        {
            dirs[i++] = e->d_name;
            print(buf_for_print_rights, e->d_name)
            e = readdir(d);
        }
        else
        {
            print(buf_for_print_rights, e->d_name)
            e = readdir(d);
        }
    }
    printf("\n");
    for (int j = 0; j < i; j++)
    {
        char *path_end = strchr(buf, '\0');
        sprintf(path_end, "%s/", dirs[j]);
        print_dir_rec(opendir(buf), dirs[j], buf, is_a, is_i, is_l, is_n);
        *path_end = '\0';
    }
}

int compare(const void *lhs, const void *rhs)
{
    return ((struct argument *)lhs)->type > ((struct argument *)rhs)->type;
}

struct argument* sort_arg(struct argument *arg, int arg_count)
{
    qsort(arg, arg_count, sizeof(struct argument), compare);
    return arg;
}

int main(int argc, char *argv[])
{
    int c = 0;
    int is_l = 0, is_R = 0, is_a = 0, is_d = 0, is_n = 0, is_i = 0;
    while((c = getopt(argc, argv, "lRadni")) != -1)
    {
        switch(c)
        {
            case 'l':
            {
                is_l = 1;
                break;
            }
            case 'a':
            {
                is_a = 1;
                break;
            }
            case 'd':
            {
                is_d = 1;
                break;
            }
            case 'n':
            {
                is_n = 1;
                break;
            }
            case 'i':
            {
                is_i = 1;
                break;
            }
            case 'R':
            {
                is_R = 1;
                break;
            }
            default:
            {
                printf("ERROR in getopt\n");
                exit(1);
            }
        }
    }

    int opt_count = is_l + is_R + is_a + is_d + is_n + is_i;
    int arg_count = argc - opt_count - 1;
    struct argument *arg = (struct argument*)calloc(arg_count, sizeof(struct argument));
    for (int i = 0; i < arg_count; ++i)
    {
        if (optind < argc)
        {
            struct stat statbuf = {};
            if(stat(argv[optind + i], &statbuf) < 0)
            {
                printf("No such file or directory: %s\n", argv[optind + i]);
                exit(0);
            }
            arg[i].name = argv[optind + i];
            switch (statbuf.st_mode & S_IFMT)
            {
                case S_IFREG:
                {
                    arg[i].type = 0;
                    break;
                }
                case S_IFDIR:
                {
                    arg[i].type = 1;
                    break;
                }
            }
        }
    }

    if (arg_count == 0)
    {
        DIR *d = opendir(".");
        char buf[512] = {};
        sprintf(buf, "%s/", ".");
        if(is_R)
            print_dir_rec(d, ".", buf, is_a, is_i, is_l, is_n);
        else
            print_dir(d, is_a, is_i, is_l, is_n);
        closedir(d);

    }
    else
    {
        if (arg_count > 1)
            arg = sort_arg(arg, arg_count);

        for (int i = 0; i < arg_count; ++i)
        {
            if (arg[i].type == 0)
            {
                printf("%s\n\n", arg[i].name);
            }
            else
            {
                char buf[512] = {};
                sprintf(buf, "%s/", arg[i].name);
                DIR *d = opendir(arg[i].name);
                if (is_R)
                    print_dir_rec(d, arg[i].name, buf, is_a, is_i, is_l, is_n);
                else
                    print_dir(d, is_a, is_i, is_l, is_n);
                printf("\n");
                closedir(d);
            }
        }
    }
    free(arg);
    return 0;
}
