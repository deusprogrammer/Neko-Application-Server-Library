AR := ar cr
GXX := g++
SRC  := nekoServer.cpp simSock.cpp thread.cpp timer.cpp strutils.cpp
FLAGS := -w -c

all: neko.a

nekoServer.o: nekoServer.cpp
	$(GXX) $(FLAGS) nekoServer.cpp

simSock.o: simSock.cpp
	$(GXX) $(FLAGS) simSock.cpp

thread.o: thread.cpp
	$(GXX) $(FLAGS) thread.cpp

timer.o: timer.cpp
	$(GXX) $(FLAGS) timer.cpp

strutils.o: strutils.cpp
	$(GXX) $(FLAGS) strutils.cpp

neko.a: nekoServer.o simSock.o thread.o strutils.o
	$(AR) neko.a *.o

clean:
	rm -rf *.o *.a
