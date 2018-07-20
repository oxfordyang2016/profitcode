#include "zshared/socket.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include <cstring>
#include <stdexcept>
#include <string>

namespace zshared {

void Socket::Bind(const std::string &ip, int port) {
  ConvertAddress(ip, port);
  if (bind(socket_,
           reinterpret_cast<const sockaddr*>(&socket_address_),
           sizeof(socket_address_)) < 0) {
    throw std::runtime_error(BuildError("bind: "));
  }
}

void Socket::Connect(const std::string &ip, int port) {
  ConvertAddress(ip, port);
  if (connect(socket_,
              reinterpret_cast<const sockaddr*>(&socket_address_),
              sizeof(socket_address_)) < 0) {
    throw std::runtime_error(BuildError("connect: "));
  }
}

ssize_t Socket::Receive(void *data, size_t length, int flags) {
  return recv(socket_, data, length, flags);
}

ssize_t Socket::Send(const void *data, size_t length) {
  return send(socket_, data, length, 0);
}

void Socket::SetReuseAddr() {
  const int on = 1;
  int res = setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  if (res < 0) {
    throw std::runtime_error(BuildError("setsockopt: "));
  }
}

void Socket::SetNonBlocking() {
  int flags = fcntl(socket_, F_GETFL, 0);
  if (flags < 0) {
    throw std::runtime_error(BuildError("fcntl get: "));
  }

  flags |= O_NONBLOCK;
  int res = fcntl(socket_, F_SETFL, flags);
  if (res < 0) {
    throw std::runtime_error(BuildError("fcntl set: "));
  }
}

void Socket::SetReceiveBufferSize(int receive_buffer_size) {
  int n = receive_buffer_size;
  if (setsockopt(socket_, SOL_SOCKET, SO_RCVBUF, &n, sizeof(n)) < 0) {
    throw std::runtime_error(BuildError("setsockopt: "));
  }
  socklen_t s = sizeof(n);
  getsockopt(socket_, SOL_SOCKET, SO_RCVBUF, &n, &s);
  if (n < receive_buffer_size) {
    throw std::runtime_error("Error, could not set SO_RCVBUF large enough.  "
                             "Check sysctl network settings!");
  }
}

int Socket::GetDescriptor() const {
  return socket_;
}

void Socket::Initialize(int socket_type) {
  socket_ = socket(AF_INET, socket_type, IPPROTO_IP);
  if (socket_ < 0) {
    throw std::runtime_error(BuildError("socket: "));
  }
}

void Socket::SetDescriptor(int descriptor) {
  socket_ = descriptor;
}

void Socket::ConvertAddress(const std::string &ip, int port) {
  socket_address_.sin_family = AF_INET;
  socket_address_.sin_port = htons(port);

  if (ip != kSocketAnyAddress) {
    if (!inet_aton(ip.c_str(), &address_)) {
      throw std::runtime_error("Unable to convert ip " + ip + " to binary");
    }
    socket_address_.sin_addr.s_addr = address_.s_addr;
  } else {
    socket_address_.sin_addr.s_addr = htonl(INADDR_ANY);
  }
}

bool Socket::WaitForData(int seconds, int useconds) {
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(socket_, &readfds);

  // check return_time
  timeval timeout;
  timeout.tv_sec = seconds;
  timeout.tv_usec = useconds;
  int rv = select(socket_+1, &readfds, NULL, NULL, &timeout);
  if (rv < 0) {
    throw std::runtime_error("select: " + std::string(strerror(errno)));
  }
  if (rv == 0) {
    // timed out
    return false;
  }
  return true;
}

//////////////////////////////////////////

std::string Socket::BuildError(const std::string &error) {
  return error + strerror(errno);
}


UdpSocket::UdpSocket() {
  Initialize(SOCK_DGRAM);
}

ssize_t UdpSocket::SendMulticast(void *data, size_t length) {
  return sendto(
      socket_,
      data,
      length,
      0,
      reinterpret_cast<const sockaddr*>(&socket_address_),
      sizeof(socket_address_));
}

void UdpSocket::ListenMulticast(
    const std::string &multicast_address,
    const std::string &interface_address) {
  ip_mreq request;
  in_addr converted;

  if (!inet_aton(multicast_address.c_str(), &converted)) {
    throw std::runtime_error("inet_aton failed for multicast_address");
  }

  request.imr_multiaddr.s_addr = converted.s_addr;

  if (interface_address != kSocketAnyAddress) {
    if (!inet_aton(interface_address.c_str(), &converted)) {
      throw std::runtime_error("inet_aton failed for interface_address");
    }
    request.imr_interface.s_addr = converted.s_addr;
  } else {
    request.imr_interface.s_addr = htonl(INADDR_ANY);
  }

  // Join mcast group
  if (setsockopt(socket_, IPPROTO_IP, IP_ADD_MEMBERSHIP, &request, sizeof(request)) < 0) {
    throw std::runtime_error(BuildError("Error joining group: "));
  }
}

void UdpSocket::EnableMulticastBroadcast(const std::string &multicast_address, const int port) {
  in_addr address;
  address.s_addr = INADDR_ANY;
  if (setsockopt(socket_, IPPROTO_IP, IP_MULTICAST_IF, &address, sizeof(address)) < 0) {
    throw std::runtime_error(BuildError("Error enabling multicast"));
  }

  in_addr converted;
  if (!inet_aton(multicast_address.c_str(), &converted)) {
    throw std::runtime_error("inet_aton failed");
  }

  socket_address_.sin_family = AF_INET;
  socket_address_.sin_addr.s_addr = converted.s_addr;
  socket_address_.sin_port = htons(port);
}

ssize_t UdpSocket::ReceiveFrom(void *data, size_t length, int flags, sockaddr* from, socklen_t* from_len) {
  return recvfrom(socket_, data, length, flags, from, from_len);
}

TcpSocket::TcpSocket() {
  Initialize(SOCK_STREAM);
}

void TcpSocket::SetNoDelay() {
  const int on = 1;
  int res = setsockopt(socket_, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
  if (res < 0) {
    throw std::runtime_error(BuildError("setsockopt: "));
  }
}


TcpSocket TcpSocket::Accept() {
  listen(socket_, 5);

  sockaddr_in address;
  socklen_t size = sizeof(address);
  int descriptor = accept(socket_, reinterpret_cast<sockaddr*>(&address), &size);

  TcpSocket socket;
  socket.SetDescriptor(descriptor);
  return socket;
}

void TcpSocket::ReceiveAll(void* data, size_t length, int flags) {
  char* ptr = static_cast<char*>(data);
  size_t total_bytes_read = 0;
  do {
    ssize_t bytes_read = Receive(ptr + total_bytes_read, length - total_bytes_read, flags);
    if (bytes_read < 0) {
      throw std::runtime_error("ReceiveAll error: " + std::string(strerror(errno)));
    }
    if (bytes_read == 0) {
      throw std::runtime_error("ReceiveAll empty read");
    }

    total_bytes_read += bytes_read;
    if (total_bytes_read > length) {
      // This should never happen if we did the math correctly above
      throw std::runtime_error("Logic error!  Received too many bytes");
    }
  } while (total_bytes_read != length);
}

}  // namespace zshared
