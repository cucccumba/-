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
#include <time.h>

#define MIN_DIRS_SIZE 100
#define MAX_PATH_SIZE 1024

struct argument
{
    char *name;
    int type;  //! 0: file, 1: directory
    ino_t ino;
};


char * print_rights(char *buf, char *name, int is_n)
{
    struct stat statbuf = {};
    if (lstat(buf, &statbuf) < 0)
        perror("ERROR in print_rights");
    int is_link = 0;
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
            is_link = 1;
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
        char* uname = getpwuid(statbuf.st_uid)->pw_name;
        char* gname = getgrgid(statbuf.st_gid)->gr_name;
        printf("%s %s ", uname, gname);
    }

    printf("%ld ", statbuf.st_size);

    char *time = ctime(&(statbuf.st_ctim.tv_sec));
    char *endl = strchr(time, '\n');
    *endl = '\0';
    printf("%s ", time);
    if (is_link)
    {
        char *link = (char *)calloc(256, sizeof(char));
        readlink(buf, link, 256);
        return link;
    }
    else
        return NULL;
}

void print_info (char *buf, char *name, ino_t ino, int is_i, int is_l, int is_n)
{
    char *link = NULL;
    if (is_i)
        printf("%lu ", ino);
    if (is_l || is_n)
        link = print_rights(buf, name, is_n);
    printf("%s", name);
    if (link)
        printf("  ->  %s", link);
    printf("\n");
}

void print_dir(char *buf, char *name, ino_t ino, int is_a, int is_i, int is_l, int is_n, int is_d)
{
   DIR *d = opendir(buf);

    if (is_d)
        print_info(name, name, ino, is_i, is_l, is_n);
    else
    {
        struct dirent *e = readdir(d);
        while (e != NULL)
        {
            if (is_a)
                print_info(name, e->d_name, e->d_ino, is_i, is_l, is_n);
            else
            {
                if (e->d_name[0] != '.')
                    print_info(name, e->d_name, e->d_ino, is_i, is_l, is_n);
            }
            e = readdir(d);
        }
    }
}

char * print_dir_rec(char *path, char *dir_name, ino_t dir_ino, int is_a, int is_i, int is_l, int is_n, int is_d)
{
    DIR *d = opendir(path);
    if (d == NULL)
    {
        perror("ERROR in opendir");
        exit(1);
    }
    struct dirent *e = readdir(d);

    char **dirs = (char **)calloc(MIN_DIRS_SIZE, sizeof(char *));
    for (int i = 0; i < MIN_DIRS_SIZE; ++i)
        dirs[i] = (char *)calloc(256, sizeof(char));
    size_t dirs_size = MIN_DIRS_SIZE;

    printf("%s:\n", dir_name);
    int j = 0;
    while (e != NULL)
    {
        if (strcmp(e->d_name, ".") != 0 && strcmp(e->d_name, "..") != 0)
        {
            struct stat statbuf = {};
            char buf[MAX_PATH_SIZE] = {};
            memcpy(buf, path, MAX_PATH_SIZE);
            char *name = e->d_name;
            strcat(buf, name);

            ino_t ino;
            if (lstat(buf, &statbuf) < 0)
                perror("ERROR in statbuf in rec");

            if (is_d)
            {
                if (is_a)
                    print_info(buf, e->d_name, e->d_ino, is_i, is_l, is_n);
                else
                {
                    if (e->d_name[0] != '.')
                        print_info(buf, e->d_name, e->d_ino, is_i, is_l, is_n);
                }
            }
            else
            {
                if (e->d_type == DT_DIR)
                {
                    if (dirs_size < j + 1)
                    {
                        dirs = (char **)realloc(dirs, (dirs_size + 100) * sizeof(char *));
                        for (size_t i = dirs_size; i < dirs_size + 100; ++i)
                            dirs[i] = (char *)calloc(256, sizeof(char));
                        dirs_size += 100;
                    }
                    if (is_a)
                    {
                        print_info(buf, e->d_name, e->d_ino, is_i, is_l, is_n);
                        memcpy(dirs[j++], e->d_name, 256);
                    }
                    else
                    {
                        if (e->d_name[0] != '.')
                        {
                            print_info(buf, e->d_name, e->d_ino, is_i, is_l, is_n);
                            memcpy(dirs[j++], e->d_name, 256);
                        }
                    }
                }
                else
                {
                    if (is_a)
                        print_info(buf, e->d_name, e->d_ino, is_i, is_l, is_n);
                    else
                    {
                        if (e->d_name[0] != '.')
                            print_info(buf, e->d_name, e->d_ino, is_i, is_l, is_n);
                    }
                }
            }
        }
        e = readdir(d);
    }
    printf("\n");
    for (int i = 0; i < j; ++i)
    {
        strcat(path, dirs[i]);
        strcat(path, "/");
        print_dir_rec(path, dirs[i], dir_ino, is_a, is_i, is_l, is_n, is_d);
        char *path_end = strstr(path, dirs[i]);
        char *p =path_end;
        while (p != NULL)
        {
            path_end = p;
            p  = strstr(path_end + strlen(dirs[i]), dirs[i]);
        }
        *(path_end) = '\0';
    }
    closedir(d);
    for (size_t i = 0; i < dirs_size; ++i)
        free(dirs[i]);
    free(dirs);
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
            arg[i].ino = statbuf.st_ino;
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
        struct stat statbuf = {};
        if (stat(".", &statbuf) < 0)
            perror("ERROR in stat");
        char buf[512] = {};
        sprintf(buf, "%s/", ".");
        sprintf(buf + 2, "%c", '\0');
        if(is_R)
        {
            print_dir_rec(buf, ".", statbuf.st_ino, is_a, is_i, is_l, is_n, is_d);
        }
        else
            print_dir(buf, ".", statbuf.st_ino, is_a, is_i, is_l, is_n, is_d);
        return 0;
    }
    else
    {
        if (arg_count > 1)
            arg = sort_arg(arg, arg_count);

        for (int i = 0; i < arg_count; ++i)
        {
            if (arg[i].type == 0)
            {
                print_info(arg[i].name, arg[i].name, arg[i].ino, is_i, is_l, is_n);
            }
            else
            {
                char buf[512] = {};
                sprintf(buf, "%s", arg[i].name);
                DIR *d = opendir(arg[i].name);
                if (is_R)
                   print_dir_rec(buf, arg[i].name, arg[i].ino, is_a, is_i, is_l, is_n, is_d);
                else
                    print_dir(buf, arg[i].name, arg[i].ino, is_a, is_i, is_l, is_n, is_d);
                printf("\n");
                closedir(d);
            }
        }
    }
    free(arg);
    return 0;
}
