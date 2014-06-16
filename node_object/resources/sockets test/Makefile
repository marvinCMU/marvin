# Makefile for the socket programming example
#

commServer_objects = ServerSocket.o Socket.o commServer_main.o


all : commServer

commServer: $(commServer_objects)
	g++ -o commServer $(commServer_objects) -lpthread


Socket: Socket.cpp
ServerSocket: ServerSocket.cpp
commServer_main: commServer_main.cpp


clean:
	rm -f *.o commServer
