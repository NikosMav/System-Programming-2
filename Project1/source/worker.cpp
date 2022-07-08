// manager.cpp: the parent program

#include "../headers/aux.h"

// SIGTERM signal handler
volatile sig_atomic_t sigterm_flag = 0;
void sigterm_handler(int signum)
{
    sigterm_flag = 1;
}

int main(int argc, char **argv) {

    // Set up a signal handler for when user interrupts the programm 
    struct sigaction sigterm_act;
    memset(&sigterm_act, 0, sizeof sigterm_act);
    sigemptyset(&sigterm_act.sa_mask);
    sigterm_act.sa_handler = sigterm_handler;       // Catch the SIGTERM
    // Catching SIGTERM
    if(sigaction(SIGTERM, &sigterm_act, NULL) != 0)
    {
        perror("sigaction () failed installing SIGTERM handler");
        return EXIT_FAILURE;
    }

    // Set up a signal handler for when user interrupts the programm 
    struct sigaction sigint_act;
    memset(&sigint_act, 0, sizeof sigint_act);
    sigemptyset(&sigint_act.sa_mask);
    sigint_act.sa_handler = SIG_IGN;            // Ignore the SIGINT
    sigint_act.sa_flags = SA_RESTART;
    // Catching SIGINT
    if(sigaction(SIGINT, &sigint_act, NULL) != 0)
    {
        perror("sigaction () failed installing SIGINT handler");
        return EXIT_FAILURE;
    }

    int fd;
    // Get the worker's pipe name
    string pipe_name = argv[1];
    string *filename = new string();
    while(1) { // BIG LOOP

        if(sigterm_flag) {
            delete filename;

            cout << "child: " << getpid() << " exited successfully" << endl;
            exit(EXIT_SUCCESS);
        }

        //cout << "WORKER " << pipe_name << " STARTED!" << endl;

        // Open named pipe and read file
        fd = open(pipe_name.c_str(), O_RDONLY);
        if (fd ==-1) { 
            perror("Error");
            exit(EXIT_FAILURE);
        }

        *filename = readFileNameFromPipe(fd);
        //cout << "file received: " << *filename << endl;

        if (close(fd) == -1)
        {
            perror("Error");
            exit(EXIT_FAILURE);
        }

        // Find urls from file
        map<string, int> list_of_urls = extractURLs(filename);

        //printMap(list_of_urls);

        // Create new file and write results
        writeURLs(list_of_urls, filename);

        //cout << "WORKER " << pipe_name << " ENDED!" << endl;

        // Send signal SIGSTOP after work is done
        if(raise(SIGSTOP) != 0)
        {
            perror("Can't raise SIGSTOP");
            exit(EXIT_FAILURE);
        }

    }
}