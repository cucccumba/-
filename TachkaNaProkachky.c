#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
// 0 sem - gate 1
// 1 sem - gate 2
// 2 sem - master
// 3 sem - ready or not
struct cars_num
{
    int free_park_lot;
    int cars_for_repair;
};

int master(int id, struct cars_num *ptr)
{
    struct sembuf master_sleep = {2, -1, 0};
    struct sembuf let_go = {3, 1, 0};
    int cars_repaired = 0;
    printf("Master was born\n");
    while (1)
    {
        if(semop(id, &master_sleep, 1) < 0)
            perror("ERROR in master gate open");

        printf("Master is repairing car\n");
        sleep(1); //repairing car

        if (semop(id, &let_go, 1) < 0)
            perror("ERROR in let_go");
        cars_repaired++;
        if(cars_repaired == ptr->cars_for_repair)
        {
            return 0;
        }
    }
}

int client(int id, int j, struct cars_num *ptr)
{
    // car is entering parking lot
    printf("Car %d entering parking\n", j);

    struct sembuf gate_open = {0, -1, 0};
    struct sembuf gate_close = {0, 1, 0};
    //Start of critical section
    if (semop(id, &gate_open, 1) < 0)
        perror("ERROR in gate_open");
    if (ptr->free_park_lot)
    {
        --(ptr->free_park_lot);
        ++(ptr->cars_for_repair);
        printf("Car %d on parking\n", j);
    }
    else
    {
        printf("Car %d went away\n", j);
        return 0;
    }
    if (semop(id, &gate_close, 1) < 0)
        perror("ERROR in gate_close");
    // End os critical section

    //car is going to the master
    struct sembuf gate2_open = {1, -1, 0};
    struct sembuf gate2_close = {1, 1, 0};
    struct sembuf awake = {2, 1, 0};
    struct sembuf leave = {3, -1, 0};
    //critical section
    if (semop(id, &gate2_open, 1) < 0)
        perror("ERROR in semop gate2_open");
    printf("Car %d is at the master\n", j);
    ++(ptr->free_park_lot);
    if (semop(id, &awake, 1) < 0)
        perror("ERROR in awake");
    if (semop(id, &leave, 1) < 0)
        perror("ERROR in leave");
    printf("Car %d left the master\n", j);
    if (semop(id, &gate2_close, 1) < 0)
        perror("ERROR in gate2_close");
    //critical section
    return 0;
}

int main(int argc, char * argv[])
{
    int parkSize = atoi(argv[1]);
    int carNum = atoi(argv[2]);

    int id = semget(IPC_PRIVATE, 4, IPC_CREAT | 0777);
    int idshm = shmget(IPC_PRIVATE, sizeof(struct cars_num), IPC_CREAT | 0777);
    struct cars_num *ptr = (struct cars_num *) shmat(idshm, NULL, 0);
    ptr->free_park_lot = parkSize;
    ptr->cars_for_repair = 0;
    
    struct sembuf initGate = {0, 1, 0};
    struct sembuf initGate2 = {1, 1, 0};
    if(semop(id, &initGate, 1) < 0)
    perror("Error in init gate");
    if (semop(id, &initGate2, 1) < 0)
    perror("ERROR in init Gate2");

    if(id < 0)
    {
        perror("Error in semget");
        exit(1);
    }
    if(fork() == 0)
    {
        master(id, ptr);
        return 0;
    }
    for(int j = 0; j < carNum; ++j)
    {
        if(fork() == 0)
        {
            client(id, j + 1, ptr);
            return 0;
        }
    }   
    for(int k = 0; k < carNum; ++k)
        wait(NULL);
    if(semctl(id, 0, IPC_RMID, 0) < 0)
        perror("Error in semctl ");
    printf("Master went home\n");
    return 0;
}
