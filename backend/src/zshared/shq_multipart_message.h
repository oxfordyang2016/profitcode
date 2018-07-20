#ifndef SRC_ZSHARED_SHQ_MULTIPART_MESSAGE_H_
#define SRC_ZSHARED_SHQ_MULTIPART_MESSAGE_H_

#include <stdint.h>
#include <shq/shq.h>
#include <vector>

namespace zshared {

class Header;

class MultipartMessageBuilder {
 public:
  MultipartMessageBuilder();

  bool AddMessage(const void* data, size_t size);
  const void* GetData();
  uint16_t GetSize();

 private:
  Header* GetHeader();

  std::vector<char> bytes_;
};

class MultipartMessageReader {
 public:
  MultipartMessageReader();
  bool Initialize(const void* data, size_t size);

  bool GetNextMessage(const void** data, size_t* size);
  uint16_t GetNumMessages() const;

 private:
  const Header* header_;
  uint32_t offset_;
};
}

#endif  // SRC_ZSHARED_SHQ_MULTIPART_MESSAGE_H_
