GXX := g++
LIBS := -lpthread -lssl
SRC  := nekoServer.cpp simSock.cpp thread.cpp timer.cpp strutils.cpp
FLAGS := -w -c

all: 
	$(GXX) $(FLAGS) $(SRC) $(LIBS)

clean:
	rm -rf *.o
