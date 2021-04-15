

#ifndef SAF_MESSAGE_H
#define SAF_MESSAGE_H

#include "ns3/packet.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"

#include <stdio.h>
#include <string.h>
#include <vector>

#include "data.h"

namespace saf {
using namespace ns3;

enum class MessageType : uint8_t { unknown = 0x00, lookup = 0x01, dataResponse = 0x02 };

class Message {
 private:
  virtual std::vector<uint8_t> GeneratePayload();

 protected:
  /* data */
  uint32_t m_request_id;
  uint32_t m_response_id;
  int64_t m_requested_at;
  int64_t m_sent_at;
  MessageType m_type;

 public:
  Message();
  ~Message();
  Ptr<Packet> ToPacket();
};

class LookupMessage : public Message {
 private:
  uint16_t m_data_id;
  bool m_is_replication;
  std::vector<uint8_t> GeneratePayload();

 public:
  LookupMessage(uint16_t dataID, bool isReplication);
};

class ResponseMessage : public Message {
 private:
  uint16_t m_data_id;  // the data object this is a response for
  // dont need to actually have the data conent here
  uint32_t m_data_size;
  bool m_is_replication;
  std::vector<uint8_t> GeneratePayload();

 public:
  ResponseMessage(uint32_t requestID, uint64_t requestedAt, bool isReplication, Data data);
};

}  // namespace saf

#endif /*  SAF_DATA_H */
