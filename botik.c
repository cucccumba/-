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
//! 2 sem - unloading capacity
//! 3 sem - trap for unload
void ship(int capacity, int cruises, int sem_id)
{

    struct sembuf init_cap = {1, capacity, 0};
    struct sembuf open_trap = {0, 1, 0};
    struct sembuf close_trap[] = {{1, 0, 0}, {0, -1, 0}};
    struct sembuf open_trap_1 = {3, 1, 0};
    struct sembuf close_trap_1[] = {{2, 0, 0}, {3, -1, 0}};
    if (semop(sem_id, &init_cap, 1) < 0)
        perror("ERROR in init_cap");
    for (int i = 0; i < cruises; ++i)
    {
        printf("Ship is ready to load %d time\n", i + 1);
        if (semop(sem_id, &open_trap, 1) < 0)
            perror("ERROR in open_trap");
        if (semop(sem_id, close_trap, 2) < 0)
            perror("ERRRO in close_trap");

        printf("Ship went %d time\n", i + 1);
        sleep(1);
        printf("Ship arrived\n");

        if (semop(sem_id, &open_trap_1, 1) < 0)
            perror("ERROR in open_trap_1");
        if (semop(sem_id, close_trap_1, 2) < 0)
            perror("ERROR in close_trap_1");
    }
    printf("Ship ended its work\n");
    
    if (semctl(sem_id, 0, IPC_RMID, 0) < 0)
        perror("ERROR in semctl");
}

void passenger(int i, int sem_id)
{
    struct sembuf enter_trap[] = {{0, -1, 0}, {1, -1, 0}};
    struct sembuf get_place[] = {{2, 1, 0}, {0, 1, 0}};
    struct sembuf enter_trap_1[] = {{3, -1, 0}, {2, -1, 0}};
    struct sembuf free_place[] = {{1, 1, 0}, {3 , 1, 0}};
    
    printf("Zelibobka %d on beach\n", i + 1);
    while (1)
    {
        if (semop(sem_id, enter_trap, 2) < 0)
            exit(0);
        printf("Zelibobka %d on ship\n", i + 1);
        if (semop(sem_id, get_place, 2) < 0)
            perror("ERROR in get_place");
        
        if (semop(sem_id, enter_trap_1, 2) < 0)
            perror("ERROR in enter_trap_1");
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

    int sem_id = semget(IPC_PRIVATE, 4, IPC_CREAT | 0777);
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
