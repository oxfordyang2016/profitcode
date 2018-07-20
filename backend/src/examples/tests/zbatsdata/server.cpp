#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>

#include "zshared/socket.h"
#include "market_handlers/zbatsdata/messages.h"

std::vector<bats::AddOrderLongMessage> active_orders;
std::vector<bool> active_orders_alive;
std::vector<uint32_t> active_order_seq_no;
uint64_t current_time = 0;
uint32_t g_sequence_no = 1;

int g_unit, g_port;

class Builder {
 public:
  explicit Builder(bool isSequenced) {
    bytes_ = new char[1500];
    header_ = reinterpret_cast<bats::SequencedUnitHeader*>(bytes_);
    header_->unit = g_unit;
    header_->sequence_no = isSequenced ? g_sequence_no : 0;
    header_->message_count = 0;

    length_ = sizeof(bats::SequencedUnitHeader);
  }

  ~Builder() {
    delete[] bytes_;
  }

  void Send(zshared::TcpSocket *socket) {
    header_->length = length_;
    socket->Send(bytes_, length_);
  }

  void Send(zshared::UdpSocket *socket) {
    if (header_->sequence_no != 0) {
      g_sequence_no += header_->message_count;
    }

    header_->length = length_;
    socket->SendMulticast(bytes_, length_);
  }

  void ReAddOrder(bats::AddOrderLongMessage* order) {
    bats::AddOrderLongMessage *message =
      MakeMessage<bats::AddOrderLongMessage>(bats::kMessageTypeAddOrderLong);

    memcpy(message, order, sizeof(bats::AddOrderLongMessage));
  }

  void AddOrder(bool is_bid, uint32_t size, bats::Symbol symbol, uint64_t price) {
    bats::AddOrderLongMessage *message =
      MakeMessage<bats::AddOrderLongMessage>(bats::kMessageTypeAddOrderLong);
    message->ns = (current_time % 1000) * 1000;
    message->order_id = active_orders.size();
    message->side = is_bid ? 'B' : 'S';
    message->size = size;
    message->symbol = symbol;
    message->price.price = price;
    message->flags = 0;

    active_orders.push_back(*message);
    active_orders_alive.push_back(true);
    active_order_seq_no.push_back(header_->sequence_no);
  }

  void DeleteOrder(uint64_t order_id) {
    active_orders_alive[order_id] = false;

    bats::DeleteOrderMessage *message =
      MakeMessage<bats::DeleteOrderMessage>(bats::kMessageTypeDeleteOrder);
    message->ns = (current_time % 1000) * 1000;
    message->order_id = order_id;
  }

  void SpinAvailable(uint32_t sequence_no) {
    bats::SpinImageAvailableMessage *message =
      MakeMessage<bats::SpinImageAvailableMessage>(bats::kMessageTypeSpinImageAvailable);
    message->sequence_no = sequence_no;
  }

  void SpinResponse(
      uint32_t sequence_no,
      uint32_t order_count,
      char status) {
    bats::SpinResponseMessage *message =
      MakeMessage<bats::SpinResponseMessage>(bats::kMessageTypeSpinResponse);
    message->order_count = order_count;
    message->sequence_no = sequence_no;
    message->status = status;
  }

  void SpinFinished(uint32_t sequence_no) {
    bats::SpinFinishedMessage *message =
      MakeMessage<bats::SpinFinishedMessage>(bats::kMessageTypeSpinFinished);
    message->sequence_no = sequence_no;
  }

  void LoginResponse(char status) {
    bats::LoginResponseMessage *message =
      MakeMessage<bats::LoginResponseMessage>(bats::kMessageTypeLoginResponse);
    message->status = status;
  }

  void TimeMessage() {
    bats::TimeMessage* message =
      MakeMessage<bats::TimeMessage>(bats::kMessageTypeTime);

    time_t now;
    time(&now);
    time_t midnight = now / 86400 * 86400;
    time_t midnight_est = midnight + (5 * 60 * 60);

    message->time = now - midnight_est;
  }

 private:
  template<class MessageType>
  MessageType *MakeMessage(const bats::kMessageTypes message_type) {
    MessageType *message = reinterpret_cast<MessageType*>(bytes_ + length_);
    message->header.length = sizeof(MessageType);
    message->header.message_type = message_type;

    length_ += sizeof(MessageType);
    header_->message_count++;

    return message;
  }

  char *bytes_;
  size_t length_;
  bats::SequencedUnitHeader *header_;
};

void *SpinAdvertise(void *arg) {
  zshared::TcpSocket *socket = static_cast<zshared::TcpSocket*>(arg);

  for (;;) {
    // Broadcast what we can send the spins for
    Builder builder(false);
    builder.SpinAvailable(active_orders.size());
    builder.Send(socket);

    sleep(1);
  }
  return 0;
}

void *SpinServer(void *arg) {
  zshared::TcpSocket acceptor;
  acceptor.SetReuseAddr();
  acceptor.Bind("127.0.0.1", g_port + 1);

  printf("Spin server started...\n");

  zshared::TcpSocket socket = acceptor.Accept();
  printf("Client connected to spin server\n");
  char raw_bytes[1500];
  socket.Receive(raw_bytes, sizeof(raw_bytes), 0);
  printf("Login from:\n");

  {
    Builder builder(false);
    builder.LoginResponse('A');
    builder.Send(&socket);
  }

  // Create thread to send out advertisements
  pthread_t advertise_thread;
  pthread_create(&advertise_thread, 0, SpinAdvertise, &socket);

  // Now, listen for user requests
  for (;;) {
    bats::SequencedUnitHeader container_header;
    if (socket.Receive(&container_header, sizeof(container_header), 0) !=
        sizeof(container_header)) {
      printf("recv error\n");
      break;
    }
    if (socket.Receive(raw_bytes, container_header.length - sizeof(container_header), 0) !=
        ssize_t(container_header.length - sizeof(container_header))) {
      printf("recv error\n");
      break;
    }

    int offset = 0;
    for (int i = 0; i < container_header.message_count; i++) {
      const bats::Message *raw_message = reinterpret_cast<const bats::Message*>(raw_bytes + offset);
      offset += raw_message->header.length;

      switch (raw_message->header.message_type) {
        case bats::kMessageTypeSpinRequest:
        {
          const bats::SpinRequestMessage *message =
            reinterpret_cast<const bats::SpinRequestMessage*>(raw_message);

          size_t total_orders = active_orders.size();
          uint32_t order_count = 0;
          for (size_t j = 0; j < total_orders; j++) {
            if (active_orders_alive[j] &&
                active_order_seq_no[j] < message->sequence_no) {
              ++order_count;
            }
          }

          printf("Got spin request for %d, sending %d orders\n",
            message->sequence_no,
            order_count);

          {
            Builder builder(false);
            builder.SpinResponse(message->sequence_no, order_count, 'A');
            builder.Send(&socket);
          }

          for (size_t j = 0; j < total_orders; j++) {
            if (active_orders_alive[j] &&
                active_order_seq_no[j] < message->sequence_no) {
              printf("Sending order %zu\n", active_orders[j].order_id);
              Builder builder(false);
              builder.ReAddOrder(&active_orders[j]);
              builder.Send(&socket);
            }
          }

          {
            Builder builder(false);
            builder.SpinFinished(message->sequence_no);
            builder.Send(&socket);
          }
        }
        break;
      }
    }
  }

  return 0;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("%s unit port_number\n", argv[0]);
  }

  g_unit = atoi(argv[1]);
  g_port = atoi(argv[2]);

  active_orders.push_back(bats::AddOrderLongMessage());
  active_orders_alive.push_back(false);
  active_order_seq_no.push_back(0);

  pthread_t thread;
  pthread_create(&thread, 0, SpinServer, 0);

  zshared::UdpSocket socket;
  socket.EnableMulticastBroadcast("226.0.0.1", g_port);

  bats::Symbol symbols[5];
  std::string raw_symbols[5] = { "$_AB", "$_AC", "$_AD", "$_AE", "$_AF" };

  for (int i = 0; i < 5; i++) {
    symbols[i] = bats::CreateSymbol(raw_symbols[i]);
  }

  for (int i = 0; i < 100; i++) {
    usleep(100000);

    Builder builder(true);
    builder.TimeMessage();
    builder.AddOrder(true, 100, symbols[0], 52000);
    builder.AddOrder(true, 90, symbols[0], 53000);
    for (uint64_t j = 0; j < active_orders_alive.size(); j++) {
      if (active_orders_alive[j]) {
        builder.DeleteOrder(j);
        break;
      }
    }
    builder.Send(&socket);
  }

  return 0;
}
