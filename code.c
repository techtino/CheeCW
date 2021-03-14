#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <Python.h>
#include <sys/stat.h>

//Global variable declaration
int fd[2], int_caught, tstp_caught, child_complete;
void child_sig_handler(int signo);
void parent_sig_handler(int signo);
void create_status_file(int pid);
void generateNumber();

int main() {
        int pid;
        //Ensuring the pipe is created successfully
        if (pipe(fd) < 0)
                printf("Pipe Error");

        //Signal handlers per signal are set
        signal(SIGINT,child_sig_handler);
        signal(SIGTSTP,child_sig_handler);
        signal(SIGCHLD, parent_sig_handler);

        //Ensure fork is successful
        if((pid = fork()) < 0){
                printf("Fork error\n");
        }

        //Parent code
        else if (pid > 0){
                close(fd[1]); //close write end of pipe
                dup2(fd[0], fileno(stdin)); //read end of pipe bound to stdin
                create_status_file(pid);
                while(child_complete != 1){} //halts program whilst child not complete

                //Variables for reading line by line
                char *line;
                size_t bufsize = 2;
                int i=0;
                char **array;
                array = malloc(bufsize * sizeof(char*)); //allocates required memory to array pointer
                
                //While line isnt empty, copy the number into array
                while ((getline(&line, &bufsize, stdin)) != -1) {
                        array[i] = malloc(strlen(line) + 1); //allocates extra memory for new line
                        strcpy(array[i], line); //copies string into array memory location
                        i++;
                } 
                close(fd[0]); //close read end of pipe
        }

        //Child code
        else{
                close(fd[0]); //close read end of pipe
                dup2(fd[1], fileno(stdout)); //stdout bound to write end of pipe
                while(int_caught != 1){ //Code runs whilst CTRL-C has not been pressed
                        //Check for CTRL-Z to be pressed and generates number
                        if(tstp_caught == 1){
                                tstp_caught = 0;
                                generateNumber();
                        }
                }
        }
}

void child_sig_handler (int signo){
        switch(signo){
                case 2:
                        int_caught = 1; //Sets variable to inform that ctrl+c was pressed
                        break;
                case 20:
                        tstp_caught = 1; //Inform program that ctrl-z was pressed       
                        break;
                default:
                        printf("Unknown");
        }
}

void parent_sig_handler(int signo){
        child_complete = 1; // inform program that child has finished
}

void create_status_file(int pid){
        char *file = "status.txt";
        
        //Creating structure of current system time
        time_t T= time(NULL);
        struct tm tm = *localtime(&T);

        pid_t parentPID = getpid(); // Gets parent PID
        
        int statusDescriptor = open(file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR); //status file opened for write as file descriptor fd
        dup2(statusDescriptor, fileno(stdout)); //stdout file descriptor contents are duped to status file desciptor
       
        // Make use of stat command to get information about inode
        struct stat file_stat;
        int inode;
        fstat (statusDescriptor, &file_stat);

        printf("Program ran at: %02d:%02d:%0.2d\n",tm.tm_hour,tm.tm_min,tm.tm_sec); //time is printed to file
        printf("Child PID is %d, the parent PID is %d\n", pid, parentPID);
        printf("Inode Number of file: %d, User ID of owner: %d, Group ID: %d, Date Created (unix timestamp): %d", file_stat.st_ino, file_stat.st_uid, file_stat.st_gid, file_stat.st_ctim);
        close(statusDescriptor);
}

void generateNumber(){
        srand(time(0));
        int num = (rand() % (50 - 10 + 1)) + 10;
        printf("%d\n", num);
}