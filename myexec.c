#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int fd[2];
    int ok = pipe(fd);
    if (ok < 0)
        perror("ERROR");
    
    struct timespec tp;
    int ook = clock_gettime(CLOCK_MONOTONIC, &tp);
    int sec1 = tp.tv_sec;
    long nsec1 = tp.tv_nsec;

    if (argc > 1)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            if(close(1) < 0)
                perror("ERROR");
            if(dup(fd[1]) < 0)
                perror("ERROR");
            if(close(fd[1]))
                perror("ERROR");
            
            execvp(argv[1], argv + 1);
        }
    }
    wait(NULL);

    char buf[1024] = {};
    int byte_count = 0;
    int lines_count = 0;
    int words_count = 0;
    byte_count = read(fd[0], buf, 1024);

    for (int i = 0; i < 1024; ++i)
    {
        if (buf[i] == '\n')
             ++lines_count;
        if (buf[i] == ' ')
            ++words_count;
    }

    ook = clock_gettime(CLOCK_MONOTONIC, &tp);
    int sec2 = tp.tv_sec;
    long nsec2 = tp.tv_nsec;
    
    printf("Time: %.2f ms\n", (sec2 * 1000 + nsec2 / 1000000.0 - sec1 * 1000 - nsec1 / 1000000.0));
    printf("Lines count: %d, words count: %d, bytes count: %d\n", lines_count, words_count + 1, byte_count);
    return 0;
}
