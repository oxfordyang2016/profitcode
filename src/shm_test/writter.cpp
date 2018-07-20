#include <sys/shm.h>
#include <sys/time.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

struct shm_content {
  char data[32];
  bool is_good;
};

int main() {
  int shmid = shmget((key_t)1234, sizeof(struct shm_content), 0666|IPC_CREAT);
  if (shmid == -1) {
    printf("Create failed!\n");
    exit(1);
  }
  void* shm = NULL;
  shm = shmat(shmid, 0, 0);
  if (shm == NULL) {
    printf("shmat failed!\n");
    exit(1);
  }
  struct shm_content* content = (struct shm_content*)shm;
  while (true) {
    if (content->is_good == true) {
      printf("\nenter the string:");
      std::string a;
      std::cin >> a;
      snprintf(content->data, sizeof(content->data), "%s", a.c_str());
      timeval time;
      gettimeofday(&time, NULL);
      printf("write %s, time is %ld %ld\n", content->data, time.tv_sec, time.tv_usec);
      content->is_good = false;
    }
  }
}
