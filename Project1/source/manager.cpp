// manager.cpp: the parent program
#include "../headers/aux.h"

// Global variable for naming the named pipes of workers
int global_id = 0;

// Setting up the worker queue globally
queue<QueueInfo *> worker_queue;

//////////////////////////////////////// SIGNAL HANDLER FUNCTIONS ////////////////////////////////////////
// SIGCHLD signal handler
void sigchld_handler(int signum)
{
    pid_t stopped_child;
    int   status;
    QueueInfo *stopped_worker_info;
    while ((stopped_child = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) // find which child exited
    {
        // It means that it is done with its work and can be used again.
        // Now find pid in worker queue...
        while(!worker_queue.empty()) {
            stopped_worker_info = worker_queue.front();
            if(stopped_worker_info->getPid() == stopped_child) {
                // change workers availability
                stopped_worker_info->setIsAvailable(true);
                break;

            }
            worker_queue.pop();
            worker_queue.push(stopped_worker_info);
        }
    }     
}

// SIGINT signal handler
volatile sig_atomic_t sigint_flag = 0;
void sigint_handler(int signum)
{
    sigint_flag = 1;
}

//////////////////////////////////////// MAIN CODE ////////////////////////////////////////
int main(int argc, char **argv){

    // Set up a signal handler for when children processes exit
    struct sigaction sigchld_act;
    memset(&sigchld_act, 0, sizeof sigchld_act);
    sigemptyset(&sigchld_act.sa_mask);
    sigchld_act.sa_handler = sigchld_handler;
    sigchld_act.sa_flags = SA_RESTART;
    // Catching SIGCHLD
    if(sigaction(SIGCHLD, &sigchld_act, NULL) != 0)
    {
        perror("sigaction () failed installing SIGCHLD handler");
        return EXIT_FAILURE;
    }

    // Set up a signal handler for when user interrupts the programm 
    struct sigaction sigint_act;
    memset(&sigint_act, 0, sizeof sigint_act);
    sigemptyset(&sigint_act.sa_mask);
    sigint_act.sa_handler = sigint_handler;
    sigint_act.sa_flags = 0;
    // Catching SIGINT
    if(sigaction(SIGINT, &sigint_act, NULL) != 0)
    {
        perror("sigaction () failed installing SIGINT handler");
        return EXIT_FAILURE;
    }

    // Set up a signal handler for when user interrupts the programm 
    struct sigaction sigterm_act;
    memset(&sigterm_act, 0, sizeof sigterm_act);
    sigemptyset(&sigterm_act.sa_mask);
    sigterm_act.sa_handler = SIG_IGN;            // Ignore the SIGINT
    sigterm_act.sa_flags = SA_RESTART;
    // Catching SIGINT
    if(sigaction(SIGTERM, &sigterm_act, NULL) != 0)
    {
        perror("sigaction () failed installing SIGINT handler");
        return EXIT_FAILURE;
    }

    // Check for main arguments
    string monitored_path;
    if(argc == 3){
        monitored_path = argv[2];
    } else if(argc == 1) {
        monitored_path = "./";
    } else {
        cout << "Error: Wrong number of arguments" << endl;
        exit(EXIT_FAILURE);
    } 

    // listener communicates with manager and informs him about new events
    int listener_manager[2];
    if (pipe(listener_manager) == -1) {
        fprintf(stderr, "Pipe Failed");
        return 1;
    }
    // Fork to create listener...
    pid_t listener_pid = fork();
    if (listener_pid < 0) {
        fprintf(stderr, "fork Failed");
        return 1;
    }
    /////////////////////////////////// LISTENER PROCESS ///////////////////////////////////
    else if (listener_pid == 0){
        // no need to use the read end of pipe here so close it
        close(listener_manager[0]);
        // closing the standard output
        close(1);
        // duplicating fd[0] with standard output 1
        dup(listener_manager[1]);

        // Get arguments for calling inotifywait function on "files" directory
        char const *argv_list[] = {"inotifywait", "-m", monitored_path.c_str(), "-e", "close_write", "-e" , "moved_to", NULL};
        execvp(argv_list[0], const_cast<char**>(argv_list));
    }
    /////////////////////////////////// MANAGER PROCESS ///////////////////////////////////
    // closing the standard input 
    close(0);
    // no need to use the write end of pipe here so close it
    close(listener_manager[1]);
    // duplicating fd[0] with standard input 0
    dup(listener_manager[0]);

    // Initialization of variables before loop
    QueueInfo *avail_worker_info;
    string *read_string = new string();
    string *filename = new string();
    int fdout;
    string pipe_name;
    global_id = 0;

    while(1){ // BIG LOOP

        // Get the file name, programm pauses here.
        *read_string = readFileNameFromPipe(listener_manager[0]);

        // Behaviour after signal handler of SIGINT
        if(sigint_flag) {
            // wait for listener to terminate
            int listener_status;
            waitpid(listener_pid, &listener_status, WNOHANG);

            // Send signal to all child process and terminate them
            terminateWorkers(worker_queue);

            // Free the allocated memory of queue
            freeQueue(worker_queue);

            // Collect parent's garbage
            delete read_string;
            delete filename;

            cout << endl << "EXITED SUCCESSFULLY" << endl;
            exit(EXIT_SUCCESS);
        }

        //showq(worker_queue);

        // Get the actual file's name
        *filename = extractFileName(read_string);
        cout << "new file detected: " << *filename << endl;

        // Check the worker queue for available workers
        if((worker_queue.empty()) || !(isAvailableWorker(worker_queue))) { // if queue is empty or there are no available workers then...

            // Creating the named pipe file(FIFO) mkfifo(<pathname>, <permission>)
            global_id++;
            pipe_name = "/tmp/fifo" + to_string(global_id);
            mkfifo(pipe_name.c_str(), 0666);

            // Fork and create worker
            pid_t new_worker_pid = fork();
            if (new_worker_pid < 0) {
                fprintf(stderr, "fork Failed");
                return 1;

            } else if (new_worker_pid == 0) {  //Child process - WORKER
                char const *argv_list[] = {"./worker", pipe_name.c_str(), NULL};
                execvp(argv_list[0], const_cast<char**>(argv_list));
            }

            // Add new worker to queue
            // Create an instance of its info
            QueueInfo *new_worker_info = new QueueInfo(new_worker_pid, false, pipe_name);
            // And push it to queue
            worker_queue.push(new_worker_info);

        } else {     // This means that there is at least one available worker, we need to find an available one...

            // choose the first out available worker from queue
            avail_worker_info = findAvailableWorker(worker_queue);
            // Worker is no longer available
            avail_worker_info->setIsAvailable(false);
            // gets its pipe name
            pipe_name = avail_worker_info->getPipeName();
            // Send signal SIGCONT to available worker to continue
            kill(avail_worker_info->getPid(), SIGCONT);
        }

        // Manager writes filename to child
        fdout = open(pipe_name.c_str(), O_WRONLY);
        if (fdout ==-1) { 
            perror("Error");
            exit(EXIT_FAILURE);
        }

        writeFileNameToPipe(fdout, filename);

        if(close(fdout) == -1) {
            perror("Error");
            exit(EXIT_FAILURE);
        }
    }
}