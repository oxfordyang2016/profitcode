#include <sys/time.h>
#include <zmq.hpp>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include <string>
#include <iostream>

pthread_mutex_t mutex;

int flag = 0;
zmq::context_t context(1);
zmq::socket_t socket(context, ZMQ_REQ);

/*
void timer_handler(int a) {
  if (flag == 1) {
    cout << "jump out\n";
    return;
  }
  char hb[3] = "HB";
  zmq::message_t msg(hb, 3, NULL);
  socket.send(msg);
  cout << "HB send " << hb << "  " << "\n";
  socket.recv(&msg);
  cout << "HB recv " << reinterpret_cast<char*>(msg.data()) << "  "  <<"\n";
}
*/

int main() {
  /*
  struct itimerval t;
  t.it_interval.tv_usec = 0;
  t.it_interval.tv_sec = 10;
  t.it_value.tv_usec = 0;
  t.it_value.tv_sec = 3;
  // setitimer(ITIMER_REAL, &t, NULL);
  signal(SIGALRM, timer_handler);
  */
  socket.connect("tcp://52.196.151.242:21212");
  while (true) {
    // i++;
    /*
    cout << i << "\n";
    if (i > 15 && i < 30) {
      sleep(2);
      continue;
    }
    */

    char str[5] ="hehe";
    try {
    pthread_mutex_lock(&mutex);
    zmq::message_t msg(str, sizeof(str), NULL);
    socket.send(msg);
    std::cout << "send " << str << "   " << " " << "\n";
    zmq::message_t re;
    socket.recv(&re);
    std::cout << "recv " << reinterpret_cast<char*>(re.data()) <<"\n";
    pthread_mutex_unlock(&mutex);
    } catch (zmq::error_t error) {
      printf("hehe\n");
    }
    sleep(2);
  }
  return 0;
}
