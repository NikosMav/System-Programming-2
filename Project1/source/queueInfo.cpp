#include "../headers/queueInfo.h"

QueueInfo::QueueInfo(pid_t pid, bool is_available, string pipe_name)
{
    this->pid = pid;
    this->is_available = is_available;
    this->pipe_name = pipe_name;
}
// Default constructor
QueueInfo::QueueInfo(){
    this->pid = 0;
    this->is_available = false;
    this->pipe_name = "";
}
// Copy constructor
QueueInfo::QueueInfo(const QueueInfo &info) {
    this->pid = info.pid;
    this->is_available = info.is_available;
    this->pipe_name = info.pipe_name;
}
QueueInfo::~QueueInfo(){}
void QueueInfo::setPid(pid_t pid) { this->pid = pid; }
void QueueInfo::setIsAvailable(bool is_available) { this->is_available = is_available; }
void QueueInfo::setPipeName(string pipe_name) { this->pipe_name = pipe_name; }
pid_t QueueInfo::getPid() { return this->pid; }
bool QueueInfo::getIsAvailable() { return this->is_available; }
string QueueInfo::getPipeName() { return this->pipe_name; }