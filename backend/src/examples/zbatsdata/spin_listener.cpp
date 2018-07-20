#include "market_handlers/zbatsdata/spin_listener.h"

#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>

#include <algorithm>
#include <cmath>
#include <string>

#include "zshared/socket.h"
#include "common/zentero_util.h"
#include "common/logging/topic_logger.h"
#include "market_handlers/zbatsdata/message_processor.h"
#include "market_handlers/zbatsdata/tcp_server.h"
#include "market_handlers/zbatsdata/unit.h"

namespace bats {

static void SendSequencedUnit(const Unit &unit,
                              zshared::TcpSocket* socket,
                              void* message,
                              size_t length) {
  char raw_bytes[150];
  SequencedUnit* container = reinterpret_cast<SequencedUnit*>(raw_bytes);

  ssize_t container_length = sizeof(SequencedUnit) + length;
  container->header.length = container_length;
  container->header.message_count = 1;
  container->header.unit = unit.GetId();
  container->header.sequence_no = 0;
  memcpy(container->data, message, length);

  if (container_length != socket->Send(container, container_length)) {
    throw std::runtime_error("Unable to send sequenced unit");
  }
}

// note: header->unit always set to 0 for bats so no need to check it
static void ReadSequencedUnit(zshared::TcpSocket* socket,
                              SequencedUnitHeader* header,
                              void* data,
                              size_t length) {
  size_t expected_length = 0;
  // read header and get expected_length
  socket->ReceiveAll(header, sizeof(SequencedUnitHeader), 0);
  if (header->length < sizeof(SequencedUnitHeader)) {
    throw std::runtime_error("negative expected_length");
  }
  expected_length = header->length - sizeof(SequencedUnitHeader);
  if (length < expected_length) {
    throw std::runtime_error("Buffer too small");
  }

  // read expected_length bytes into data
  socket->ReceiveAll(data, expected_length, 0);
}

static void Login(const Unit &unit,
                  zshared::TcpSocket* socket,
                  const TcpServer & spin_server,
                  TopicLogger* topic_logger) {
  socket->Connect(spin_server.GetAddress(), spin_server.GetPort());

  LoginMessage login_message = spin_server.BuildLoginMessage();
  const LoginResponseMessage* login_response;

  SendSequencedUnit(unit, socket, &login_message, sizeof(login_message));

  // Login response is the only expected response
  SequencedUnitHeader header;
  char raw_bytes[150];
  ReadSequencedUnit(socket, &header, raw_bytes, sizeof(raw_bytes));

  if (header.message_count != 1) {
    throw std::runtime_error("Too many messages received in login response");
  }

  login_response = reinterpret_cast<const LoginResponseMessage*>(raw_bytes);
  if (login_response->status == 'A') {
    topic_logger->Log("INFO", "Logged in to spin server");
    return;
  }

  // Failed login
  if (login_response->status == 'B') {
    throw std::runtime_error("Spin server login failed - busy");
  } else {
    char buf[250];
    snprintf(buf, sizeof(buf), "Spin server login failed with status == %c",
      login_response->status);
    throw std::runtime_error(buf);
  }
}

uint32_t SpinListener::RebuildBook(const Unit & unit,
                                   const TcpServer & spin_server,
                                   uint32_t start_sequence_no,
                                   MessageProcessor* processor,
                                   TopicLogger* topic_logger) {
  // don't send out market updates while rebuilding book
  processor->SetPublishEnabled(false);

  zshared::TcpSocket socket;
  Login(unit, &socket, spin_server, topic_logger);

  uint32_t final_sequence_no = 0;
  char bytes[8192];

  bool finished = false;
  while (!finished) {
    SequencedUnitHeader container_header;
    ReadSequencedUnit(&socket, &container_header, bytes, sizeof(bytes));

    // Go through all the messages, and update the state
    int offset = 0;
    for (int i = 0; i < container_header.message_count; i++) {
      const Message* raw_message = reinterpret_cast<const Message*>(bytes + offset);
      offset += raw_message->header.length;

      switch (raw_message->header.message_type) {
        case kMessageTypeSpinImageAvailable:
          {
            const SpinImageAvailableMessage* message =
                reinterpret_cast<const SpinImageAvailableMessage*>(raw_message);

            topic_logger->Log("INFO", "Spin available: %d - need %d",
                              message->sequence_no,
                              start_sequence_no);

            // Do we need to start a spin - and is the spin sequence high enough?
            if (final_sequence_no == 0 &&
                message->sequence_no >= start_sequence_no) {
              final_sequence_no = message->sequence_no;

              // Send the request to start the spin now
              SpinRequestMessage spin_request;
              spin_request.header.length = sizeof(SpinRequestMessage);
              spin_request.header.message_type = kMessageTypeSpinRequest;
              spin_request.sequence_no = final_sequence_no;
              SendSequencedUnit(unit, &socket, &spin_request, sizeof(spin_request));

              topic_logger->Log("INFO", "Sending spin request");
            }
          }
          break;

        case kMessageTypeSpinResponse:
          {
            const SpinResponseMessage *message =
                reinterpret_cast<const SpinResponseMessage*>(raw_message);

            topic_logger->Log("INFO", "Spin response from sequence %d, with status '%c'",
                              message->sequence_no,
                              message->status);
          }
          break;

        case kMessageTypeSpinFinished:
          topic_logger->Log("INFO", "Spin finished received");
          finished = true;
          break;

        default:
          processor->Process(raw_message);
          break;
      }
    }
  }

  processor->SetPublishEnabled(true);
  return final_sequence_no;
}

}  // namespace bats
