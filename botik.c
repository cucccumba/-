#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>

//! 0 sem - trap for load
//! 1 sem - ship_capacity
//! 2 sem - unloading
//! 3 sem - trap for unload
//! 4 sem - loading
void ship(int capacity, int cruises, int sem_id)
{

    struct sembuf init_capacity = {1, capacity, 0};
    if (semop(sem_id, &init_capacity, 1) < 0)
        perror("ERROR in init_capacity");
    struct sembuf init_unloading = {2, capacity, 0};
    struct sembuf open_trap_1 = {0, 1, 0};
    struct sembuf open_trap_2 = {3, 1, 0};
    struct sembuf load[2] = {{4, 0, 0}, {0, -1, 0}};
    struct sembuf unload[2] = {{2, 0, 0}, {3, -1, 0}};
    for (int i = 0; i < cruises; ++i)
    {
        printf("Ship is ready to load %d'st time\n", i + 1);
        struct sembuf init_loading = {4, capacity, 0};
        if (semop(sem_id, &init_loading, 1) < 0)
            perror("ERROR in init_loading");
        if (semop(sem_id, &open_trap_1, 1) < 0)
            perror("ERROR in open_trap");
        if (semop(sem_id, load, 2) < 0)
            perror("ERROR in load");
        printf("Ship left\n");
        
        sleep(1);
        
        printf("Ship arrived to unload\n");
        if (semop(sem_id, &init_unloading, 1) < 0)
            perror("ERROR in init_unloading");
        if (semop(sem_id, &open_trap_2 , 1) < 0)
            perror("ERROR in open_trap");
        if (semop(sem_id, unload, 2) < 0)
            perror("ERROR in unload");
    }
    printf("Ship ended its work\n");
    
    if (semctl(sem_id, 0, IPC_RMID, 0) < 0)
        perror("ERROR in semctl");
}

void passenger(int i, int sem_id)
{
    struct sembuf pass_trap_1[2] = {{0, -1, 0}, {0, 1, 0}};
    struct sembuf pass_trap_2[2] = {{3, -1, 0}, {3, 1, 0}};
    struct sembuf get_place = {1, -1, 0};
    struct sembuf free_place[2] = {{1, 1, 0}, {2, -1, 0}};
    struct sembuf load = {4, -1, 0};
    while (1)
    {
        printf("Zelibobka %d on beach\n", i + 1);
        if (semop(sem_id, &get_place, 1) < 0)
        {
            //perror("ERROR in get_place");
            //exit(0);
        }
        if (semop(sem_id, pass_trap_1, 2) < 0)
        {
            //perror("ERROR in pass_trap_1");
            exit(0);
        }
        if (semop(sem_id, &load, 1) < 0)
            perror("ERROR in load");
        printf("Zelibobka %d on ship\n", i + 1);
        if (semop(sem_id, pass_trap_2, 2) < 0)
            perror("ERROR in pass_trap_2");
        printf("Zelibobka %d left ship\n", i + 1);
        if (semop(sem_id, free_place, 2) < 0)
            perror("ERROR in free_place");
    }
}

int main(int argc, char *argv[])
{
    int capacity = atoi(argv[1]);
    int pass_num = atoi(argv[2]);
    int cruises =  atoi(argv[3]);

    int sem_id = semget(IPC_PRIVATE, 5, IPC_CREAT | 0777);
    if (sem_id < 0)
    {
        perror("ERROR in semget");
        exit(1);
    }

    if (fork() == 0)
    {
        ship(capacity, cruises, sem_id);
        return 0;
    }

    for (int i = 0; i < pass_num; ++i)
    {
        if (fork() == 0)
        {    
            passenger(i, sem_id);
            return 0;
        }
    }
    for (int i = 0; i < pass_num + 1; ++i)
        wait(NULL);

    return 0;
}
