#include <unistd.h>
#include <stdio.h>

int main()
{
    __pid_t pid;
    int status;
    if ((pid = fork()) == 0)
    {
        printf("This is child.\n");
    }
    else 
    {
        waitpid(pid, &status ,0);
        printf("This is parent.\n");
    }

}