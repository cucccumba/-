#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>

#define MIN_PATHS_SIZE 10
#define MIN_DIRS_SIZE 100
#define MAX_PATH_SIZE 1024

struct hardlinks
{
    ino_t ino;
    char **paths;
    size_t paths_size;
    struct hardlinks *next;
    int i;
    int num_of_hardlinks;
};

struct hardlinks * init_hardlinks(struct hardlinks *h);

struct hardlinks *check_ino_add(struct hardlinks *h, ino_t ino_, char *buf, int num_links)
{
    int is_ino = 0;
    struct hardlinks *list = h;
    struct hardlinks *prev = h;
    while (h)
    {
        if (h->ino == ino_)
        {
            is_ino = 1;
            if (h->paths_size < h->i + 1)
            {
                h->paths = (char **)realloc(h->paths, (h->paths_size * 2) * sizeof(char *));
                for (size_t i = list->paths_size; i < h->paths_size * 2; ++i)
                    h->paths[i] = (char *)calloc(MAX_PATH_SIZE, sizeof(char));
                h->paths_size *= 2;
            }
            memcpy(h->paths[h->i], buf, strlen(buf));
            ++(h->i);
            break;
        }
        prev = h;
        h = h->next;
    }
    if (is_ino == 0)
    {
        prev->next = (struct hardlinks *)calloc(1, sizeof(struct hardlinks));
        prev->next = init_hardlinks(prev->next);
        prev->ino = ino_;
        prev->num_of_hardlinks = num_links;
        memcpy(prev->paths[0], buf, strlen(buf));
        prev->i = 1;
    }
    return list;
}


void check_dir_rec(char *path, struct hardlinks *h)
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


            if (statbuf.st_nlink > 1 && e->d_type != DT_DIR)
            {
                ino = statbuf.st_ino;
                h = check_ino_add(h, ino, buf, statbuf.st_nlink);
            }
            if (e->d_type == DT_DIR)
            {
                if (dirs_size < j + 1)
                {
                    dirs = (char **)realloc(dirs, (dirs_size + 100) * sizeof(char *));
                    for (size_t i = dirs_size; i < dirs_size + 100; ++i)
                        dirs[i] = (char *)calloc(256, sizeof(char));
                    dirs_size += 100;
                }
                memcpy(dirs[j++], e->d_name, 256);
            }

        }
        e = readdir(d);
    }
    for (int i = 0; i < j; ++i)
    {
        strcat(path, dirs[i]);
        strcat(path, "/");
        check_dir_rec(path, h);
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

struct hardlinks * init_hardlinks(struct hardlinks *h)
{
    h->i = 0;
    h->paths = (char **)calloc(MIN_PATHS_SIZE, sizeof(char*));
    for (int i = 0; i < MIN_PATHS_SIZE; ++i)
        h->paths[i] = (char *)calloc(MAX_PATH_SIZE, sizeof(char));
    h->paths_size = MIN_PATHS_SIZE;
    h->num_of_hardlinks = 0;
    h->next = NULL;
    return h;
}

int main(int argc, char *argv[])
{
    struct hardlinks h = {};
    init_hardlinks(&h);
    char *dir = "./";
    if (argc > 1)
        dir = argv[1];
    char path[MAX_PATH_SIZE] = {};
    strcat(path, dir);
    check_dir_rec(path, &h);

    if (strcmp(h.paths[0], "") != 0)
    {
        do
        {
            for (int i = 0; i < h.i; ++i)
                printf("%s\n", h.paths[i]);
            if (h.num_of_hardlinks != h.i)
                printf("...incomplete...");
            printf("\n");
            printf("\n");
            for (size_t i = 0; i < h.paths_size; ++i)
                free(h.paths[i]);
            free(h.paths);
            h = *(h.next);
        }
        while (h.next != NULL);
    }
    else
        printf("No hardlinks\n");
    return 0;
}

