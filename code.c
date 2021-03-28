#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <Python.h>
#include <sys/stat.h>

// Global variable declaration

int fd[2], int_caught, tstp_caught, child_complete, python_success, statusDescriptor, pid, dup_success, inode;
char const *file = "status.txt";
char PyCode[800], numbers[200];

// Time values for child and parent finishing
time_t T, childTime, parentTime;
struct tm st, ct, pt;

struct stat file_stat; // file_stat for getting details about status.txt

struct timeval tm; // Value time value for random number function

// Functions to use within program are declared
void child_sig_handler(int signo);
void parent_sig_handler(int signo);
void create_status_file(int pid);
void python_database_driver(char numbers[]);
void generateNumber();

// Main function that is run at program start
int main() {
        // Start time of program is stored
        T= time(NULL);
        st = *localtime(&T);

        //Ensuring the pipe is created successfully
        if (pipe(fd) < 0){
                printf("Pipe Error");
                exit(EXIT_FAILURE);
        }
        //Signal handlers per signal are set
        signal(SIGINT,child_sig_handler);
        signal(SIGTSTP,child_sig_handler);
        signal(SIGCHLD, parent_sig_handler);

        //Ensure fork is successful
        if((pid = fork()) < 0){
                printf("Fork error\n");
                exit(EXIT_FAILURE);
        }

        //Parent code
        else if (pid > 0){

                // Child specific signals are ignored by parent
                signal(SIGINT,SIG_IGN);
                signal(SIGTSTP,SIG_IGN);

                close(fd[1]); //close write end of pipe, no writing required in parent
                dup_success = dup2(fd[0], 0); //read end of pipe bound to stdin
                
                if (dup_success == -1){
                        printf("Dup2 failed");
                        exit(EXIT_FAILURE);
                }
                while(child_complete != 1){} //halts parent whilst child not complete
                
                // produce and store child finish time
                childTime= time(NULL);
                ct = *localtime(&childTime);
                
                // Read numbers into string
                scanf("%s",numbers);

                python_database_driver(numbers);
                create_status_file(pid);
                close(fd[0]); //close read end of pipe, program finished, cleanup
        }

        //Child code
        else if (pid == 0){
                signal(SIGCHLD,SIG_IGN);
                close(fd[0]); //close read end of pipe, no reading of pipe is required
                dup_success = dup2(fd[1], fileno(stdout)); //stdout bound to write end of pipe
                
                if (dup_success == -1){
                        printf("Dup2 failed");
                        exit(EXIT_FAILURE);
                }

                while(int_caught != 1){ //Code runs whilst CTRL-C has not been pressed
                        //Check for CTRL-Z to be pressed and generates number
                        if(tstp_caught == 1){
                                tstp_caught = 0;
                                generateNumber();
                        }
                }
                close(fd[1]);  //close write end of pipe, no more writing is done to pipe in child
        }
}

// Child signal handler to handle CTRL+Z and CTRL+C
void child_sig_handler (int signo){
        switch(signo){
                case SIGINT:
                        int_caught = 1; //Sets variable to inform that ctrl+c was pressed
                        break;
                case SIGTSTP:
                        tstp_caught = 1; //Inform program that ctrl-z was pressed       
                        break;
                default:
                        printf("Error: Unknown signal has been detected");
                        exit(EXIT_FAILURE);
        }
}

// Parent signal handler to handle SIGCHLD
void parent_sig_handler(int signo){
        child_complete = 1; // inform program that child has finished
}

// Creation of status file and storing data to it
void create_status_file(int pid){
        pid_t parentPID = getpid(); // Gets parent PID
        
        statusDescriptor = open(file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR); //status file opened for write as file descriptor fd
       
        if (statusDescriptor == -1){
                printf("There was an issue opening the file");
                exit(EXIT_FAILURE);
        }

        dup_success = dup2(statusDescriptor, 1); //stdout file descriptor contents are duped to status file desciptor
        
        if (dup_success == -1){
                printf("Dup2 failed");
                exit(EXIT_FAILURE);
        }

        // Make use of stat command to get information about inode
        fstat (statusDescriptor, &file_stat);

        parentTime= time(NULL);
        pt = *localtime(&parentTime);

        printf("Program ran at: %02d:%02d:%0.2d\n",st.tm_hour,st.tm_min,st.tm_sec); //time is printed to file
        printf("Child terminated at: %02d:%02d:%0.2d\n",ct.tm_hour,ct.tm_min,ct.tm_sec); //time is printed to file
        printf("Parent terminated at: %d:%d:%d\n", pt.tm_hour,pt.tm_min,pt.tm_sec); //time is printed to file
        printf("Child PID is %d, the parent PID is %d\n", pid, parentPID);
        printf("Inode Number of file: %d, User ID of owner: %d, Group ID: %d, Date Created (unix timestamp): %d", file_stat.st_ino, file_stat.st_uid, file_stat.st_gid, file_stat.st_ctim);
        
        fflush(stdout);
        fclose(stdout);
        fsync(statusDescriptor);
        close(statusDescriptor);
}

// Random number generator, to run when ctrl+z is pressed
void generateNumber(){

        // Seed srandom with gettimeofday(), produces more random numbers than time()
        struct timeval tm;
        gettimeofday(&tm, NULL);
        srandom(tm.tv_sec + tm.tv_usec * 1000000ul);
        
        int num = (rand() % (50 - 10 + 1)) + 10; // Produces random value between 10-50
        printf("%d,", num);
}

// Python code to create SQL table and store numbers
void python_database_driver(char numbers[]){
        Py_Initialize(); //Initialise Python connection

        //Python Code is stored in character array
        snprintf(PyCode,sizeof PyCode, //snprintf to insert numbers to python code

        "import mariadb" // Import mariadb library, community fork of MySQL
        "\nnumberArr='%s'" // Number array is set to C variable
        "\nnumberArr= numberArr[:-1]" //Remove stray comma from array
        "\nnumberList = list(map(int, numberArr.split(',')))" // Split the characters and produce integer list
        "\ncon = mariadb.connect(" // Connect to database server on machine
        "\n host='localhost',"
        "\n user='admin',"
        "\n password='mysqldbpass',"
        "\n database='NumberDB'"
        "\n)"
        "\ncur = con.cursor()"
        "\ncur.execute('DROP TABLE IF EXISTS Numbers')" // Delete existing table (Clear contents)
        "\ncur.execute('CREATE TABLE Numbers (id INT AUTO_INCREMENT PRIMARY KEY, number INT)')" // Create new table to store numbers

        //Loop through list and insert each number into sql table
        "\nfor number in numberList:"
        "\n\tcur.execute('INSERT INTO Numbers (number) VALUES (?)', (number,))"

        //Select all numbers from table, fetch them all and print row by row
        "\ncur.execute('SELECT number FROM Numbers')"
        "\nrows = cur.fetchall()"
        "\nprint('')"
        "\nfor row in rows:"
        "\n\tprint(row[0], end =' ')"
        , numbers); // Pass number string into PyCode

        python_success = PyRun_SimpleString(PyCode); //Run code
        
        // Checks if error occured within python code
        if (python_success == -1){
                printf("An error occured while accessing the database");
                exit(EXIT_FAILURE);
        }

        Py_Finalize(); // Close python connection
}
