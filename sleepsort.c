#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int arc, char* argv[])
{
    int n = atoi(argv[1]);
    for (int i = 0; i < n; ++i)
    {
        pid_t pid = fork();
        if (pid != 0)
        {
            sleep(1);
            printf("children pid = %d, parent pid = %d\n", getpid(), getppid());
            wait(NULL);
            break;
        }
    }
}
