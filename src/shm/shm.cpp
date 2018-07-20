#include <sys/shm.h>
#include <stdlib.h>
#include <string>
#include "shm.h"

#define MAXN 1000

Shm::Shm() {
  shared_mem = new struct shared_struct;
  shared_mem->can_write = true;
  shared_mem->can_read = false;
}

Shm::~Shm() {
  delete shared_mem;
}

void Shm::Create(std::string name) {
  int key = 0;
  for (size_t i = 0; i < name.size(); i++) {
    key += name[i] - 'a';
  }
  int shmid = shmget((key_t)key, sizeof(struct shared_struct), 0666|IPC_CREAT|IPC_EXCL);
  if (shmid == -1) {
    printf("shm created failed! please check the key\n");
    exit(1);
  }
  shared_mem = (struct shared_struct*)shmat(shmid, 0, 0);
  if (shared_mem == NULL) {
    printf("shmat failed!\n");
    exit(1);
  }
}

void Shm::Connect(std::string name) {
  int key = 0;
  for (size_t i = 0; i < name.size(); i++) {
    key += name[i] - 'a';
  }
  int shmid = shmget((key_t)key, sizeof(struct shared_struct), 0666|IPC_CREAT);
  if (shmid == -1) {
    printf("shm connect failed! please check the key\n");
    exit(1);
  }
  shared_mem = (struct shared_struct*)shmat(shmid, 0, 0);
  if (shared_mem == NULL) {
    printf("shmat failed!\n");
    exit(1);
  }
}

void Shm::Write(void* content) {
  shared_mem->can_read = false;
  while (true) {
    if (shared_mem->can_write) {
      shared_mem->content = content;
      shared_mem->can_read = true;
      printf("here is ok!\n");
      return;
    }
  }
}

void* Shm::Read() {
  shared_mem->can_write = false;
  while (true) {
    if (shared_mem->can_read) {
      void* temp = shared_mem->content;
      shared_mem->can_write = true;
      return temp;
    }
  }
}
