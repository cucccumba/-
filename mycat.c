#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void print_text(int *n, char* buf)
{
    if (*n < 0)
    {
        perror("ERROR");
        return 1;
    }   
    write(1, buf, *n);
    while (*n == 10240)
    {
        *n = read(0, buf, 10240);
        write(1, buf, *n);
    }   

}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        int n = 1;
        char buf[10240];
        while (n != 0)
        {
            n = read(0, buf, 10240);
            print_text(&n, buf);
        }
    }
    else
    {
        for(int i = 1; i < argc; ++i)
        {  
            int fd = open(argv[i], O_RDONLY);
            if (fd < 0)
            {
                perror("ERROR");
                return 1;
            }
            else
            {
                char buf[10240];
                int n = 0;
                n = read(fd, buf, 10240);
                print_text(&n, buf);
            }
            close(fd);
        }
    }
    return 0;
}

