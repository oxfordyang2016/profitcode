#ifndef SRC_ZSHARED_SOCKET_H_
#define SRC_ZSHARED_SOCKET_H_

#include <netinet/in.h>
#include <string>

namespace zshared {

const std::string kSocketAnyAddress = "ANY";

class Socket {
 public:
  void Bind(const std::string &ip, int port);
  void Connect(const std::string &ip, int port);

  ssize_t Receive(void *data, size_t length, int flags);
  ssize_t Send(const void *data, size_t length);

  void SetReuseAddr();
  void SetNonBlocking();
  void SetReceiveBufferSize(int receive_buffer_size);
  int GetDescriptor() const;

  // returns true if data is ready (otherwise returns false)
  bool WaitForData(int seconds, int useconds);

 protected:
  Socket() {}

  void Initialize(int socket_type);
  void SetDescriptor(int descriptor);
  void ConvertAddress(const std::string &ip, int port);
  std::string BuildError(const std::string &error);

 protected:
  int socket_;
  in_addr address_;
  sockaddr_in socket_address_;
};

class UdpSocket : public Socket {
 public:
  UdpSocket();

  ssize_t SendMulticast(void *data, size_t length);

  void ListenMulticast(
      const std::string &multicast_address,
      const std::string &interface_address);
  void EnableMulticastBroadcast(const std::string &multicast_address, const int port);

  ssize_t ReceiveFrom(void *data, size_t length, int flags, sockaddr* from, socklen_t* from_len);
};

class TcpSocket : public Socket {
 public:
  TcpSocket();
  TcpSocket Accept();

  void SetNoDelay();

  // A TCP receive which requires receiving exactly length bytes
  // Throws runtime_error if any errors
  void ReceiveAll(void* data, size_t length, int flags);
};
}

#endif  // SRC_ZSHARED_SOCKET_H_
