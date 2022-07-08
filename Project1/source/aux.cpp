#include "../headers/aux.h"

string extractFileName(string *big_string) {
    vector<string> strings1; //vector keeping strings separated by newline
    vector<string> strings2; //vector keeping strings separated by space
    string substring, subsubstring;

    istringstream bigstring(*big_string);
    //split big string with newline delimiter
    while (getline(bigstring, substring, '\n')) {
        strings1.push_back(substring);
    }

    istringstream smallstring(strings1[0]);
    //now that we have the first line and not garbage, we split the wanted line in tokens with space delimiter
    while(getline(smallstring, subsubstring, ' ')) {
        strings2.push_back(subsubstring);
    }

    //and extract the actual filename
    return strings2[0] + strings2[2];
}

void writeFileNameToPipe(int fd, string *filename) {
    // Add a newline character to the end of filename
    string write_string = *filename;
    // Perform a write()
    int status = write(fd, write_string.c_str(), strlen(write_string.c_str()) + 1);
    if(status < 0) {
        perror("Error");
        exit(EXIT_FAILURE);
    }
}

string readFileNameFromPipe(int fd) {
    int read_size = 1;      // for this specific read operation use buffersize = 1 in order to distinguish between different file creations
    char buffer[read_size];
    string resultString = "";
    string newline = "\n";
    int result;

    // Until we read \n...
    while (1) {
        result = read(fd, buffer, read_size);
        // If read() gets interuppted by a SIGNAL then break the loop.
        if(errno == EINTR) {
            break;
        } 

        if(result < 0) {
            perror("Error");
            exit(EXIT_FAILURE);
        }
        buffer[read_size] = '\0';

        // if we reached eof then stop loop
        if (result == 0) {
            break;
        }

        // Construct string
        resultString += buffer;
        //cout << "STRING: " << resultString << endl;
        // And each time clear the buffer
        memset( buffer, '\0', sizeof(buffer) );

        // if we read a new line then stop loop
        if(resultString.find(newline) != string::npos){
            break;
        }
    }

    return resultString;
}

vector<string> readFromFile(int fd, int file_size) {
    char buffer[BUFFERSIZE];
    string resultString;
    int result;
    string cutString;

    // Vector for keeping strings separated by newline
    vector<string> strings;
    // Calculate how many reads() we need to perform in order to parse the whole file
    int iterations = ceil((double)file_size / (double)BUFFERSIZE);

    // Then loop and parse the file
    int i = 0;
    int read_size = 0;
    while(i < iterations) {

        // Check if this read is the last one...
        if(i == iterations - 1) {
            // Calculate the remainder to read
            read_size = file_size - i * BUFFERSIZE;
        } else {        // Else just read BUFFERSIZE bytes...
            read_size = BUFFERSIZE;
        }

        // Read from file BUFFERSIZE bytes...
        result = read(fd, buffer, read_size);
        if(result < 0) { // if error
            perror("Error");
            exit(EXIT_FAILURE);
        }
        buffer[BUFFERSIZE] = '\0';

        // Construct the string
        resultString += buffer;
        // And each time clear the buffer
        memset( buffer, '\0', sizeof(buffer) );
        // Repeat
        i++;
    }

    // Break string to sub strings with \n as delimiter
    istringstream resultStringStream(resultString);
    while (getline(resultStringStream, cutString, '\n')) {
        strings.push_back(cutString);
    }

    return strings;
}

int findFileSize(int fd) {

    struct stat statbuf;
    if (fstat(fd, &statbuf) == -1)
    {
        printf("Error");
        exit(EXIT_FAILURE);
    }

    return statbuf.st_size;
}

map<string, int> extractURLs(string *filename) {

    // empty map container for keeping url information
    map<string, int> list_of_urls;
    map<string, int>::iterator it;
    // extract urls from string using regular expression
    regex url("http://[-a-zA-Z0-9+*~()&@#%$^?#=_|!:,.;']*(?:/|[ ])");

    // Open file 
    int fd = open(filename->c_str(), O_RDONLY);
    if (fd == -1)
    {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    // First of all find file's size
    int filesize = findFileSize(fd);
    //cout << "file size: " << filesize << endl;

    // and close it
    if (close(fd) == -1)
    {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    fd = open(filename->c_str(), O_RDONLY);
    if (fd == -1)
    {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    // Extract the lines from file...
    vector<string> lines;
    lines = readFromFile(fd, filesize);

    if (close(fd) == -1)
    {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    // Then loop though lines...
    for(string line : lines) {

        smatch match;
        while (regex_search(line, match, url)) {

            for (string x : match){
                //cout << "URL FOUND: " << x << endl;
                // Clean  url from unwanted information(www, http, etc)
                cleanupURL(x);

                // Then check if url already exists in list
                it = list_of_urls.find(x);
                if(it == list_of_urls.end()) {  //if not then add it to the list
                    //cout << "URL not present in list, insert it" << endl;
                    list_of_urls.insert({x, 1});

                } else {    //if it exists then add plus one to its occurences
                    //cout << "URL present" << endl;
                    list_of_urls[x] += 1;
                }
            }

            line = match.suffix();
        }

    }

    return list_of_urls;
}

void cleanupURL(string &dirtyURL) {
    string http_prefix = "http://";
    string last_slash = "/";
    string www_prefix = "www.";
    string space = " ";
    int start_position_to_erase;

    // First of all erase the "http://" prefix
    start_position_to_erase = dirtyURL.find(http_prefix);
    dirtyURL.erase(start_position_to_erase, http_prefix.length());

    // Then check if url contains the "www" prefix and remove it
    if(dirtyURL.find(www_prefix) != string::npos){
        start_position_to_erase = dirtyURL.find(www_prefix);
        dirtyURL.erase(start_position_to_erase, www_prefix.length());
    }

    // Then check if url contains a / at its end and remove it
    if(dirtyURL.find(last_slash) != string::npos){
        start_position_to_erase = dirtyURL.find(last_slash);
        dirtyURL.erase(start_position_to_erase, last_slash.length());
    }

    // lastly check if url contains a space at its end and remove it
    if(dirtyURL.find(space) != string::npos){
        start_position_to_erase = dirtyURL.find(space);
        dirtyURL.erase(start_position_to_erase, space.length());
    }

}

void writeURLs(map<string, int> &list_of_urls, string *filename) {

    // first split the filename with "/"
    string temp_string = *filename;
    istringstream resultStringStream(temp_string);
    string cutString;
    vector<string> strings;
    while (getline(resultStringStream, cutString, '/')) {
        strings.push_back(cutString);
    }

    // Create output file
    string outputFileName = "files_output/" + strings[1] + ".out";
    int fd = open(outputFileName.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644); 
    if (fd == -1)
    { 
        perror("Error");
        exit(EXIT_FAILURE);
    }
    
    int status;
    string url;
    int occurences;
    string output_string;
    string sum_string;
    int sum = 0;
    // Iterate through map and write it to output file
    for(auto itr = list_of_urls.begin(); itr != list_of_urls.end(); ++itr) {

        // Extract the information and get the wanted string: <url> <number of occurences>
        url = itr->first;
        occurences = itr->second;
        sum += occurences;
        output_string = url + " " + to_string(occurences) + "\n";

        // and perform write()
        status = write(fd, output_string.c_str(), strlen(output_string.c_str()));
        if (status < 0) {
            perror("Error");
        }
    }

    // sum_string = "Sum of all URLs: " + to_string(sum) + "\n";
    // // and perform write()
    // status = write(fd, sum_string.c_str(), strlen(sum_string.c_str()));
    // if (status < 0) {
    //     perror("Error");
    //     exit(EXIT_FAILURE);
    // }
    
    if (close(fd) == -1) { 
        perror("Error");
        exit(EXIT_FAILURE);
    }
}

void printMap(map<string, int> &list_of_urls) {
    // prints the elements
    int sum = 0;
    for (auto itr = list_of_urls.begin(); itr != list_of_urls.end(); ++itr) {
        cout << itr->first << '\t' << itr->second << '\n';
        sum += itr->second;
    }
    cout << "Sum of all URLs: " << sum << endl;
    cout << "==========================" << endl;
}

bool isAvailableWorker(queue<QueueInfo *> worker_queue) {
    // we're passing queue by value in order to create a copy of it
    QueueInfo *worker_info;
    while(!worker_queue.empty()) {
        worker_info = worker_queue.front();
        if(worker_info->getIsAvailable()) { // if worker is available
            return true;
        }
        worker_queue.pop();
    }

    return false;
}

QueueInfo *findAvailableWorker(queue<QueueInfo *> worker_queue) {
    // we're passing queue by value in order to create a copy of it
    QueueInfo *worker_info;
    while(!worker_queue.empty()) {
        worker_info = worker_queue.front();
        if(worker_info->getIsAvailable()) { // if worker is available
            break;
        }
        worker_queue.pop();
    }
    return worker_info;
}

void showq(queue<QueueInfo *> worker_queue) {
    // we're passing queue by value in order to create a copy of it
    cout << endl << "QUEUE IS: " << endl;
    // Print the queue
    while (!worker_queue.empty()) {
        cout << worker_queue.front()->getPipeName() << endl;
        worker_queue.pop();
    }
    cout << "===============" << endl << endl;
}

void terminateWorkers(queue<QueueInfo *> worker_queue) {
    // we're passing queue by value in order to create a copy of it
    pid_t child_pid = 0;
    QueueInfo *worker_info;
    // Send SIGTERM to all children
    while (!worker_queue.empty()) {
        worker_info = worker_queue.front();
        child_pid = worker_info->getPid();

        // Send signals to child
        if(kill(child_pid, SIGTERM) == -1) {
            perror("Error");
            exit(EXIT_FAILURE);
        }

        if(kill(child_pid, SIGCONT) == -1) {
            perror("Error");
            exit(EXIT_FAILURE);
        }
        
        // Wait for child to exit
        int status;
        wait(&status);

        worker_queue.pop();
    }

    // // Do not let zombies, wait for all the children to finish
    // while(wait(NULL) > 0);
}

void freeQueue(queue<QueueInfo *> &worker_queue){
    // Now we pass by reference in order to free the queue
    QueueInfo *worker_info;
    while(!worker_queue.empty()) {
        worker_info = worker_queue.front();
        worker_queue.pop();
        delete worker_info;
    }
}