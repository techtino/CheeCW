#include <stdio.h>
#include <unistd.h>

int main() {
        int fd[2];
        int pid;
        dup2(fd[0], stdin);
        
        signal(SIGINT, INT_handler)
        //write end of pipe bound to file
        //dup2(fd[1], );
        if (pipe(fd) < 0)
                printf("Pipe Error");
        if((pid = fork()) < 0){
                printf("Fork error\n");
        }
        else if (pid > 0){
                printf("Parent\n");
        }
        else{
                printf("Child\n");
        }
}
