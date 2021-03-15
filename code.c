#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <Python.h>
#include <sys/stat.h>

//Global variable declaration
int fd[2], int_caught, tstp_caught, child_complete, saved_stdout;

struct tm st;
struct tm ct;
struct tm pt;

void child_sig_handler(int signo);
void parent_sig_handler(int signo);
void create_status_file(int pid);
void store_to_database(char numbers[]);
void generateNumber();

int main() {
        // Start time of program is stored
        time_t T= time(NULL);
        st = *localtime(&T);
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
                dup2(fd[0], 0); //read end of pipe bound to stdin
                
                while(child_complete != 1){} //halts parent whilst child not complete
                
                // produce and store child finish time
                time_t childTime= time(NULL);
                ct = *localtime(&childTime);
                
                // Read numbers into string
                char numbers[200];
                scanf("%s",numbers);

                store_to_database(numbers);
                create_status_file(pid);
                close(fd[0]); //close read end of pipe
        }

        //Child code
        else if (pid == 0){
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
        pid_t parentPID = getpid(); // Gets parent PID
        
        int statusDescriptor = open(file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR); //status file opened for write as file descriptor fd
        dup2(statusDescriptor, 1); //stdout file descriptor contents are duped to status file desciptor
       
        // Make use of stat command to get information about inode
        struct stat file_stat;
        int inode;
        fstat (statusDescriptor, &file_stat);

        time_t parentTime= time(NULL);
        pt = *localtime(&parentTime);

        printf("Program ran at: %02d:%02d:%0.2d\n",st.tm_hour,st.tm_min,st.tm_sec); //time is printed to file
        printf("Child terminated at: %02d:%02d:%0.2d\n",ct.tm_hour,ct.tm_min,ct.tm_sec); //time is printed to file
        printf("Parent terminated at: %d:%d:%d\n", pt.tm_hour,pt.tm_min,pt.tm_sec); //time is printed to file
        printf("Child PID is %d, the parent PID is %d\n", pid, parentPID);
        printf("Inode Number of file: %d, User ID of owner: %d, Group ID: %d, Date Created (unix timestamp): %d", file_stat.st_ino, file_stat.st_uid, file_stat.st_gid, file_stat.st_ctim);
        
        fflush(stdout);
        fsync(statusDescriptor);
        close(statusDescriptor);
}

void generateNumber(){

        //seed srandom with gettimeofday(), produces more random numbers than time()
        struct timeval tm;
        gettimeofday(&tm, NULL);
        srandom(tm.tv_sec + tm.tv_usec * 1000000ul);

        int num = (rand() % (50 - 10 + 1)) + 10;
        printf("%d,", num);
}

void store_to_database(char numbers[]){
        Py_Initialize();


        PyObject* numberStr = Py_BuildValue("c", numbers);



        Py_Finalize();
}