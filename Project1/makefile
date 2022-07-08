OB = lib/manager.o lib/worker.o lib/aux.o lib/queueInfo.o
OBMAN = lib/manager.o lib/aux.o lib/queueInfo.o
OBWOR = lib/worker.o lib/aux.o lib/queueInfo.o
HEADERS = headers/aux.h headers/queueInfo.h
EXECMAN = sniffer
EXECWOR = worker
CLEANFILES = files/* files_output/*
COMP = g++
FLAGS = -std=c++11 -g -Wall -c -ggdb3
#executable
all: sniffer worker

lib/manager.o: source/manager.cpp $(HEADERS)
	$(COMP) $(FLAGS) source/manager.cpp
	mv manager.o lib/manager.o

lib/worker.o: source/worker.cpp $(HEADERS)
	$(COMP) $(FLAGS) source/worker.cpp
	mv worker.o lib/worker.o

lib/aux.o: source/aux.cpp $(HEADERS)
	$(COMP) $(FLAGS) source/aux.cpp
	mv aux.o lib/aux.o

lib/queueInfo.o: source/queueInfo.cpp $(HEADERS)
	$(COMP) $(FLAGS) source/queueInfo.cpp
	mv queueInfo.o lib/queueInfo.o

sniffer: $(OBMAN)
		$(COMP) -g $(OBMAN) -o $(EXECMAN)
	
worker: $(OBWOR)
		$(COMP) -g $(OBWOR) -o $(EXECWOR)

#cleaning command
clean :
	rm -f $(OB) $(EXECMAN) $(EXECWOR) $(CLEANFILES)