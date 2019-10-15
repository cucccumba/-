#include <unistd.h>
#include <stdio.h>

int main()
{
    execlp("echo", "KEK", "hello", " world!", NULL);
    perror("ERROR");
    return 0;
}
