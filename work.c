#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/sem.h>

int main(int argc, char *argv[])
{

    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    int sec1 = tp.tv_sec;
    int nsec1 = tp.tv_nsec;
    long wrk_num = atoi(argv[1]);
    long tsk_cnt = atoi(argv[2]);
    int id = shmget(IPC_PRIVATE, sizeof(long), IPC_CREAT | 0777);
    if (id < 0)
    {
        perror("ERROR in shmget");
        exit(1);
    }
    long *p = (long *)shmat(id, NULL, 0);
    *p = 0;
    int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0777);
    struct sembuf sops = {0, 1, 0};
    if (semop(semid, &sops, 1) < 0)
        perror("ERROR IN INIT");
    for (int i = 0; i < wrk_num; ++i)
    {
        if(fork() == 0)
        {
            struct sembuf sops1 = {0, -1, 0};
            struct sembuf sops2 = {0, 1, 0};
            if (semop(semid, &sops1, 1) < 0)
                 perror("ERROR IN CHILD BEGIN");
            for (int i = 0; i < tsk_cnt; ++i)
                ++(*p);
            if (semop(semid, &sops2, 1) < 0)
                 perror("ERROR IN CHILD END");
            return 0;
        }
    }
    for (int i = 0; i < wrk_num; ++i)
        wait(NULL);
    clock_gettime(CLOCK_MONOTONIC, &tp);
    int sec2 = tp.tv_sec;
    int nsec2 = tp.tv_nsec;
    printf("Time: %.2f ms\n", (sec2 * 1000 + nsec2 / 1000000.0 - sec1 * 1000 - nsec1 / 1000000.0));
    printf("Expected: %ld\n", wrk_num * tsk_cnt);
    printf("Real: %ld\n", *p);
    
    if (semctl(semid, 0, IPC_RMID, 0) < 0)
        perror("ERROR in semctl");

    if (shmdt(p) < 0)
        perror("ERROR in shmdt");
    if (shmctl(id, IPC_RMID, NULL) < 0)
        perror("ERROR in shmctl");
    return 0;
}
