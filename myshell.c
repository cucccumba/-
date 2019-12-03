#include "stdio.h"
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define arg_num 24
#define str_size 216
#define com_num 24

struct arg
{
    char *command;
    char *com_arg[arg_num];
};

int str_parser(struct arg *arg, char *str)
{
    int is_pipe = 0;
    char *p = strchr(str, '|');
    if (p)
        is_pipe = 1;
    if (is_pipe)
    {
        int cmd_num = 0;
        int i = 0;
        char *lines[com_num] = {};
        for (lines[i] = strtok(str, "|"); lines[i]; ++i, lines[i] = strtok(NULL, "|"))
            ++cmd_num;
        for (int j = 0; j < cmd_num; ++j)
        {
            int k = 0;
            for (arg[j].com_arg[k] = strtok(lines[j], " "); arg[j].com_arg[k]; ++k, arg[j].com_arg[k] = strtok(NULL, " "));
            arg[j].command = arg[j].com_arg[0];
        }
        return cmd_num;
    }
    else
    {

        int k = 0;
        for (arg[0].com_arg[k] = strtok(str, " "); arg[0].com_arg[k]; ++k, arg[0].com_arg[k] = strtok(NULL, " "));
        arg[0].command = arg[0].com_arg[0];
        return 1;
    }
}

void Exec (struct arg *arg, int cmd_num)
{
    int fd[2];
    int in_fd = -1;
    int is_first = 1;
    int i = 0;
    for (int j = 0; j < cmd_num; ++j)
    {
        if (!is_first)
            ++i;
        if (pipe(fd) < 0)
            perror("ERROR in pipe");
        if (fork() == 0)
        {
            close(fd[0]);
            if (!is_first)
            {
                close(0);
                dup(in_fd);
                close(in_fd);
            }
            if (j != cmd_num - 1)
            {
                close(1);
                dup(fd[1]);
                close(fd[1]);
            }
            execvp(arg[i].command, arg[i].com_arg);
        }
        else
        {
            if (in_fd != -1)
                close(in_fd);
            in_fd = fd[0];
            close(fd[1]);
        }
        is_first = 0;
    }

}

int main()
{
    char str[str_size] = {};
    printf("cucccumba-GL62M-7REX& ");
    while (fgets(str, str_size, stdin) != NULL)
    {
        struct arg arg[com_num];
        *(str + strlen(str) - 1) = '\0';
        int cmd_num = str_parser(arg, str);
        Exec(arg, cmd_num);
        for (int i = 0; i < cmd_num; ++i)
            wait(NULL);
        printf("cucccumba-GL62M-7REX& ");
    }
    return 0;
}
