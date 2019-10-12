#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main()
{
    int lenght = 0;
    char string[100] = {};
    int words_count = 0;
    printf("cucccumba-GL62M-7REX& ");
    while (fgets(string, 100, stdin) != NULL)
    {
        lenght = strlen(string);
        string[lenght - 1] = '\0';
        if (strchr(string, '|'))
        {
            int comand_count = 0;
            char *comands[20];

            int i = 0;
            for (comands[i] = strtok(string, "|"); comands[i]; ++i, comands[i] = strtok(NULL, "|"))
            {
                ++comand_count;
            }

            char *words[20][20] = {}; //[number of program][words]

            for (int j = 0; j < comand_count; ++j)
            {
                i = 0;
                for (words[j][i] = strtok(comands[j], " "); words[j][i]; ++i, words[j][i] = strtok(NULL, " "))
                {
                }
            }


            pid_t pid = 0;
            int count_pipes = 0;
            count_pipes = comand_count / 2 + 1;

            int fd[2 * count_pipes];
            for (int i = 0; i < count_pipes; ++i)
            {
                if (pipe(fd + 2 * i) < 0)
                    perror("ERROR IN PIPE");
            }

            for (int i = 0, j = 1; i < comand_count; ++i, j += 2)
            {
                int prev_fd = 0;
                pid = fork();
                if (pid == 0)
                {
                    if (i == 0)
                    {
                        close (1);
                        if (dup(fd[j]) < 0)
                            perror("ERROR IN DUP i == 0");
                        close(fd[j]);
                        prev_fd = j - 1;
                    }
                    else if (i > 0 && i < comand_count - 1)
                    {
                        close (0);
                        if (dup(fd[prev_fd]) < 0)
                            perror("ERROR IN DUP i > 0 (0)");
                        close(fd[prev_fd]);

                        close (1);
                        if(dup(fd[j]) < 0)
                            perror("ERROR IN DUP i > 0 (1)");
                        close(fd[j]);
                        prev_fd = j - 1;
                    }
                    else if (i == comand_count - 1)
                    {
                        close (0);
                        if (dup(fd[prev_fd]) < 0)
                            perror("ERROR IN DUP i == comand_count - 1 (0)");
                        close(fd[prev_fd]);
                    }

                    if (execvp(words[i][0], words[i]) < 0)
                        perror("ERROR IN EXEC");
                }
            }
            for (int i = 0; i < count_pipes * 2; ++i)
                close(fd[i]);
            for (int i = 0; i < comand_count + 1; ++i)
                wait(NULL);
            printf("cucccumba-GL62M-7REX& ");
        }
        else
        {
            char *words[20];
            int i = 0;
            for (words[i] = strtok(string, " "); words[i]; ++i, words[i] = strtok(NULL, " "))
                ++words_count;
            pid_t pid = fork();
            if (pid == 0)
            {
                words[words_count] = NULL;
                execvp(words[0], words);
            }
            wait(NULL);
            printf("cucccumba-GL62M-7REX& ");
        }
    }
    return 0;
}
