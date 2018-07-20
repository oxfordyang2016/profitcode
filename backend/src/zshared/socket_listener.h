#ifndef SRC_ZSHARED_SOCKET_LISTENER_H_
#define SRC_ZSHARED_SOCKET_LISTENER_H_

#include <tr1/functional>
#include <tr1/unordered_map>

namespace zshared {

class Socket;
class SocketListener {
 public:
  SocketListener();

  typedef std::tr1::function<void (Socket *socket)> DescriptorCallback;
  void AddDescriptor(Socket *socket, DescriptorCallback callback);

  // Timeout = 0 means non-blocking
  // Timeout = -1 means wait forever
  // Timeout > 0 is number of ms to wait
  void Process(const int timeout = 0);

 private:
  int epoll_fd_;

  std::tr1::unordered_map<int, DescriptorCallback> callbacks_;
  std::tr1::unordered_map<int, Socket*> sockets_;
};
};

#endif  // SRC_ZSHARED_SOCKET_LISTENER_H_
