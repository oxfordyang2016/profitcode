#include <sys/shm.h>
#include <string>
#include <stdio.h>

struct shared_struct {
  bool can_write;
  bool can_read;
  void* content;
};

class Shm {
 public:
  Shm();
  ~Shm();

  void Create(std::string name);
  void Connect(std::string name);
  void Write(void* content);
  void* Read();

 private:
  std::string name_;
  bool is_writting;
  bool is_reading;
  struct shared_struct* shared_mem;
};
