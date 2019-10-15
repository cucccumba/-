#include <stdio.h>
#include <string.h>
int main(int argc, char* argv[])
{
    if (argc == 1)
        printf("\n");
    else if (!strcmp(argv[1], "-n"))
    {
        for (int i = 2; i < argc - 1; ++i)
            printf("%s ", argv[i]);
        if (argc != 2)
             printf("%s", argv[argc - 1]);
    }
    else if (!strcmp(argv[1], "--help") && argc == 2)
    {
        printf("Hello I'm echo\n");
    }
    else
    {
        for(int i = 1; i < argc - 1; ++i)
            printf("%s ", argv[i]);
        printf("%s", argv[argc - 1]);
        printf("\n");
    }
    return 0;
}


