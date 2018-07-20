//
//  Hello World server in C++
//  Binds REP socket to tcp://*:5555
//  Expects "Hello" from client, replies with "World"
//
#include <zmq.hpp>
#include <string>
#include <iostream>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>

#define sleep(n)    Sleep(n)
#endif
#include <sys/time.h>
#include <unistd.h>
int main () {
    //  Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REP);
    socket.bind ("tcp://*:5555");
    struct timespec tv = {1,0};
    while (true) {
        zmq::message_t request;

        //  Wait for next request from client
        socket.recv (&request);
        timeval t;
        gettimeofday(&t, NULL);
        std::cout << "Received Hello at " << t.tv_sec << " " << t.tv_usec << std::endl;

        //  Do some 'work'
		// sleep(1);
		if(nanosleep(&tv, NULL) == -1)
		{
			exit(-1);
		}
        //  Send reply back to client
        zmq::message_t reply (5);
        memcpy ((void *) reply.data (), "World", 5);
        socket.send (reply);
    }
    return 0;
}

