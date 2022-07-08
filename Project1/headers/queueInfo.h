#ifndef QUEUEINFO
#define QUEUEINFO

#include <iostream>
#include <string>

using namespace std;

class QueueInfo {
private:
    pid_t pid;
    bool is_available;
    string pipe_name;
public:
    QueueInfo();
    QueueInfo(pid_t, bool, string);
    QueueInfo(const QueueInfo &);
    ~QueueInfo();
    void setPid(pid_t);
    void setIsAvailable(bool);
    void setPipeName(string);
    pid_t getPid();
    bool getIsAvailable();
    string getPipeName();
};

#endif