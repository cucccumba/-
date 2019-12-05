#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <string.h>

void add_ingr(char *pizza, char *ingr, int sem_id, int i)
{
    struct sembuf close_gate = {i, -1, 0};
    int next_chef = i + 1;
    if (i == 4)
        next_chef = 0;
    struct sembuf open_for_next = {next_chef, 1, 0};
    // critical section
    if (semop(sem_id, &close_gate, 1) < 0)
        perror("ERROR in close_gate");
    if (strcmp(ingr, "do") == 0 || strcmp(ingr, "z") == 0)
        pizza = strcat(pizza, ingr);
    pizza = strcat(pizza, ingr);
    if (strcmp(ingr, "a") == 0)
        pizza = strcat(pizza, " ");
    if (semop(sem_id, &open_for_next, 1) < 0)
        perror("ERROR in open_for_next");
    // critical section
}

int main(int argc, char *argv[])
{
    size_t pizzas_num = atoi(argv[1]);
    char *ingrs[] = {"do", "p", "i", "z", "a"};
    int sem_id = semget(IPC_PRIVATE, 5, IPC_CREAT | 0777);
    int shm_id =  shmget(IPC_PRIVATE, 7 * pizzas_num + pizzas_num, IPC_CREAT | 0777);
    char *pizza = (char *)shmat(shm_id, NULL, 0);
    
    struct sembuf init_do = {0, 1, 0};
    if (semop(sem_id, &init_do, 1) < 0)
        perror("ERROR in init_do");
    for (int i = 0; i < pizzas_num; ++i)
    {
        for (int j = 0; j < 5; ++j)
        {
            if (fork() == 0)
            {   
                char *ingr = ingrs[j];
                add_ingr(pizza, ingr, sem_id, j);
                return 0;
            }
        }
    }
    for (int i = 0; i < 5 * pizzas_num; ++i)
        wait(NULL);
    printf("%s\n", pizza);
    
    if(semctl(sem_id, 0, IPC_RMID, 0) < 0)
        perror("ERROR in semctl");
    if (shmctl(shm_id, IPC_RMID, NULL) < 0)
        perror("ERROR in shmctl");
    return 0;
}
