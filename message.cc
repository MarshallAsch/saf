
#include "ns3/simulator.h"

#include "message.h"

/**
 * Message format:
 * | Byte  | description |
 * | ----- | ----------- |
 * | 0     | Message Type |
 * | 1...4 | data length (MSB...LSB)|
 * | 5...8 | messageID  (MSB...LSB)|
 * | 9...13 | response To  (MSB...LSB)|
 * | 14...21| requested at |
 * | 22... | payload |
 *
 * data lookup request:
 * | Byte  | description |
 * | ----- | ----------- |
 * | 0...2 | dataID      |
 *
 * payload request:
 * | Byte  | description |
 * | ----- | ----------- |
 * | 0...2 | dataID |
 * | 3...  | data contents (random byte sequence to fill network bandwidth |
 *
 *
 */

namespace ns3 {

Message::Message() {
  static uint32_t id = 1;
  m_requested_at = Simulator::Now().GetMilliSeconds();
  m_response_id = 0;
  m_request_id = id++;
  m_type = MessageType::unknown;
}

Message::~Message() { m_requested_at = 0; }

std::vector<uint8_t> Message::GeneratePayload() { return std::vector<uint8_t>(0); }

Ptr<Packet> Message::ToPacket() {
  std::vector<uint8_t> payload = GeneratePayload();
  uint32_t len = payload.size();
  uint8_t fill[len + 22];

  fill[0] = uint8_t(m_type);

  memcpy(&fill[1], &len, sizeof(len));
  memcpy(&fill[5], &m_request_id, sizeof(m_request_id));
  memcpy(&fill[9], &m_response_id, sizeof(m_response_id));
  memcpy(&fill[14], &m_requested_at, sizeof(m_requested_at));
  memcpy(&fill[22], &payload[0], len);

  // std::cout << "payload len: " << len << "\n";
  return Create<Packet>(fill, len + 22);
}

LookupMessage::LookupMessage(uint16_t dataID) {
  m_data_id = dataID;
  m_type = MessageType::lookup;
}

std::vector<uint8_t> LookupMessage::GeneratePayload() {
  std::vector<uint8_t> payload(2);

  memcpy(&payload[1], &m_data_id, sizeof(m_data_id));

  return payload;
}

ResponseMessage::ResponseMessage(uint32_t requestID, Data data) {
  m_data_id = data.GetDataID();
  m_data_size = data.GetSize();
  m_response_id = requestID;
  m_type = MessageType::dataResponse;
}

std::vector<uint8_t> ResponseMessage::GeneratePayload() {
  std::vector<uint8_t> payload(2 + m_data_size);
  memcpy(&payload[1], &m_data_id, sizeof(m_data_id));

  return payload;
}

}  // Namespace ns3
