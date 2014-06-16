#include "ServerSocket.h"
#include "SocketException.h"
#include <string>
#include <time.h>
#include <iostream>
#include <unistd.h>

using namespace std;

#define PPP 8080

//pthread_mutex_t serverLock = PTHREAD_MUTEX_INITIALIZER;
//pthread_cond_t serverCond = PTHREAD_COND_INITIALIZER;
bool updateMessage = true;
string phoneMessage = "hello android";

void *setupServer(void *arg) {
	// Cast the parameter into what is needed
	int *incoming = (int*)arg ;
	
	int threadNumber = 1;
	int outputNumber = 1;
	try {
		// Create the socket
		ServerSocket server ( PPP );
		std::cout << "Server socket at port " << PPP << " created\n";

		while (true) {

			ServerSocket new_sock;
			server.accept ( new_sock );
			cout << "thread " << threadNumber << " is connected" << endl;
			threadNumber ++; outputNumber = 1; 
			updateMessage = true;
			try {
				while (true) {
					if (updateMessage) {
						new_sock << phoneMessage;
						updateMessage = false;
					}
				}


				cout << "data " << outputNumber << " is transmitted" << endl;
				outputNumber ++;
			}
			catch ( SocketException& ) {}

		}
	} catch ( SocketException& e ) {
		std::cout << "Exception was caught:" << e.description() << "\nExiting.\n";
    }

	return NULL;
}


int main ( int argc, char* argv[] ) {
		pthread_t serverThread;
		int threadStatus;
		int value = 40;

		pthread_create(&serverThread, NULL, setupServer, &value) ;

		// main stuff

		pthread_join (serverThread, (void **)&threadStatus);

  return 0;
}


// to test, type $ telnet localhost 8080
