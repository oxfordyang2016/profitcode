#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <tr1/unordered_map>

using namespace std;

int a;
tr1::unordered_map<int, int>m;
pthread_mutex_t order_ref_mutex;
void* RunExchangeListener(void *param) {
  while (true) {
    pthread_mutex_lock(&order_ref_mutex);
    a++;
    cout << "thread" << a << endl;
    pthread_mutex_unlock(&order_ref_mutex);
  }
}

int main() {
  a =0;
  /*
  pthread_t exchange_thread;
  if (pthread_create(&exchange_thread,
                     NULL,
                     &RunExchangeListener,
                     NULL) != 0) {
    perror("pthread_create");
    exit(1);
  }
  */
  while (true) {
    pthread_mutex_lock(&order_ref_mutex);
    a++;
    cout << "main" << a << endl;
    pthread_mutex_unlock(&order_ref_mutex);
  }
}
