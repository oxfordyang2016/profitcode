#include <zmq.hpp>
#include <string>
#include <iostream>

using namespace std;
int main() {
  zmq::context_t context(1);
  zmq::socket_t socket(context, ZMQ_SUB);
  socket.connect("inproc://my_test");
  char a[32];
  while (true) {
    socket.recv(a, sizeof(a));
    std::cout << "recved " << a << endl;
  }
}
