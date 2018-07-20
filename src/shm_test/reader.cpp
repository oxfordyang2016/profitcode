#include <sys/shm.h>
#include <sys/time.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

struct shm_content {
  char content[32];
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
  content->is_good = true;
  while (true) {
    if (content->is_good != true) {
      timeval time;
      gettimeofday(&time, NULL);
      printf("shm read get %s, time is %d %d\n", content->content, time.tv_sec, time.tv_usec);
      content->is_good = true;
    }
  }
}
