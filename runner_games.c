#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

struct msgbuf
{
    long type;
};


int main(int argc, char *argv[])
{
    int runners_count = atoi(argv[1]);
    int id = msgget(IPC_PRIVATE, IPC_CREAT | 0777);
    pid_t pid = 0;
    pid = fork();
    if (pid == 0)
    {
        printf("I'm judge\n");
        struct msgbuf buf = {1};
        for (int i = 0; i < runners_count; ++i)
            msgrcv(id, &buf, 0, runners_count + 2, 0);
        
        printf("Start\n");
        buf.type = 1; 
        msgsnd(id, &buf, 0, 0);
        msgrcv(id, &buf, 0, runners_count + 1, 0);
        printf("End of race\n");
        return 0;
    }

    for (int i = 1; i < runners_count + 1; ++i)
    {
         pid = fork();
         if (pid == 0)
         {
            printf("I'm runner %d\n", i);
            struct msgbuf buf = {runners_count + 2};
            msgsnd(id, &buf, 0, 0);

            buf.type = i;
            msgrcv(id, &buf, 0, buf.type, 0);
            
            printf("I'm running number %d\n", i);
            buf.type = i + 1;
            msgsnd(id, &buf, 0, 0);
        
            return 0;
         }
    }

    for (int i = 0; i < runners_count + 1; ++i)
        wait(NULL);
    return 0;
}
