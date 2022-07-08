#ifndef AUX
#define AUX

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <bits/stdc++.h>
#include <regex>
#include <signal.h>
#include <sys/select.h>

#include "../headers/queueInfo.h"

#define BUFFERSIZE 4096

using namespace std;

string extractFileName(string *);
void writeFileNameToPipe(int, string *);
string readFileNameFromPipe(int);
vector<string> readFromFile(int, int);
map<string, int> extractURLs(string *);
void cleanupURL(string &);
void printMap(map<string, int> &);
void writeURLs(map<string, int> &, string *);
int findFileSize(int);
bool isAvailableWorker(queue<QueueInfo *>);
QueueInfo * findAvailableWorker(queue<QueueInfo *>);
void showq(queue<QueueInfo *>);
void terminateWorkers(queue<QueueInfo *>);
void freeQueue(queue<QueueInfo *> &);

#endif