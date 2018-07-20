#include <zmq.hpp>  
#include <string>  
#include <iostream>  
#include <unistd.h>
#include <sys/time.h>
  
int main ()  
{  
    //  Prepare our context and socket  
    zmq::context_t context (1);  
    zmq::socket_t socket (context, ZMQ_REQ);  
  
    std::cout << "Connecting to hello world server…" << std::endl;  
    socket.connect ("tcp://localhost:5555");  
  
    //  Do 10 requests, waiting each time for a response  
    for (int request_nbr = 0; request_nbr != 10; request_nbr++) {  
        zmq::message_t request (6);  
        memcpy ((void *) request.data (), "Hello", 5);  
        socket.send (request);  
        timeval t;
        gettimeofday(&t, NULL);
        std::cout << "Sending Hello " << request_nbr << "…at " << t.tv_sec << " " << t.tv_usec << std::endl;  
  
        //  Get the reply.  
        zmq::message_t reply;  
        socket.recv (&reply);  
        gettimeofday(&t, NULL);
    }  
    return 0;
} 
