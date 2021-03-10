#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <Python.h>


//Global variable declaration
int fd[2];
int pid;
char *file = "status.txt";
int int_caught = 0;

void child_sig_handler(int signo);
void parent_sig_handler(int signo);
void create_status_file(int pid);

int main() {
        struct sigaction sh;

        //Defining handler for signals with sigaction
        sh.sa_handler = child_sig_handler;
        sigemptyset (&sh.sa_mask);
        sh.sa_flags = 0;
        sigaction (SIGINT, &sh, NULL);
        sigaction (SIGTSTP, &sh, NULL);

        //Ensuring the pipe is created successfully
        if (pipe(fd) < 0)
                printf("Pipe Error");

        if((pid = fork()) < 0){
                printf("Fork error\n");
        }

        //Parent code
        else if (pid > 0){
                //read end of pipe bound to stdin file descriptor ID
                dup2(fd[0], fileno(stdin));
                
                create_status_file(pid);
                waitpid(pid);
        }

        //Child code
        else{
                //stdout bound to write end of pipe
                dup2(fd[1], fileno(stdout));
                while(int_caught == 0){
                        




                }
        }
}

void child_sig_handler (int signo){
        switch(signo){
                case 2:
                        int_caught = 1;
                        break;
                case 20:
                        printf("STOP");
                        break;
                default:
                        printf("Unknown");
        }

}
void parent_sig_handler(int signo){
        printf("hello");
}


void create_status_file(int pid){
        //Creating structure of current system time
        time_t T= time(NULL);
        struct tm tm = *localtime(&T);
        
        //status file opened for write as file descriptor fd
        int statusDescriptor = open(file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);                
                
        //stdout file descriptor contents are duped to status file desciptor
        dup2(statusDescriptor, fileno(stdout));

        //time is printed to file
        printf("Program ran at: %02d:%02d:%0.2d\n",tm.tm_hour,tm.tm_min,tm.tm_sec);

        //parent and child PIDs written to file        
        pid_t parentPID = getpid();
        printf("Child PID is %d, the parent PID is %d", pid, parentPID);       
        close(statusDescriptor);
}