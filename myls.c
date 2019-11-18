#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <assert.h>

void print_dir(DIR *d)
{
    assert(d);

    struct dirent *e = readdir(d);
    while (e != NULL)
    {
        if (strcmp(e->d_name, ".") != 0 && strcmp(e->d_name, "..") != 0)
        {
            printf("%s\n", e->d_name);
        }
        e = readdir(d);
    }
}

void print_dir_rec(DIR *d, char* dir_name, char *buf)
{
    assert(d);
    char *dirs[1024] = {};
    int i = 0;
    struct dirent *e = readdir(d);
    printf("%s:\n", dir_name);
    while (e != NULL)
    {
        if (e->d_type == DT_DIR && strcmp(e->d_name, ".") != 0 && strcmp(e->d_name, "..") != 0)
        {
            dirs[i++] = e->d_name;
            e = readdir(d);
        }
        else
        {
            if (strcmp(e->d_name, ".") != 0 && strcmp(e->d_name, "..") != 0)
            {
                printf("%s\n", e->d_name);
            }
            e = readdir(d);
        }
    }
    for (int j = 0; j < i; j++)
    {
        char *path_end = strchr(buf, '\0');
        sprintf(path_end, "%s/", dirs[j]);
        //printf("---------------%s--------------\n", buf);
        printf("\n");
        print_dir_rec(opendir(buf), dirs[j], buf);
        *path_end = '\0';
        e = readdir(d);
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        DIR *d = opendir(".");
        print_dir(d);
        closedir(d);
    }
    else if (argc == 2 && argv[1][0] != '-')
    {
        char *str = argv[1];
        DIR *d = opendir(str);
        print_dir(d);
        closedir(d);
    }
    else if (argc > 1)
    {
        char *flag = argv[1];
        char *str = NULL;
        if (argc > 2)
            str = argv[2];
        
        if (strcmp(flag, "-R") == 0)
        {
            if (argc == 2)
            {
                str = ".";
            }
            DIR *d = opendir(str);
            char buf[512] = {};
            sprintf(buf, "%s/", str);
            print_dir_rec(d, str, buf);
        }
    }
    return 0;
}
