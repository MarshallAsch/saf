
#include "message.h"


namespace ns3 {

Message::Message() {
    m_requested_at = 0;
    m_requested_by = 0;
}

Message::Message(double seconds, StringValue mac) {
    m_requested_at = seconds;
    m_requested_by = 0;
}

Message::~Message() {
    m_requested_by = 0;
    m_requested_at = 0;
}

LookupMessage::LookupMessage(uint16_t dataID) {
    m_data_id = dataID;
}

Ptr<Packet>
LookupMessage::ToPacket()
{
    Ptr<Packet> p;

    uint8_t fill[10];

    fill[0] = 0x01;
    memcpy(fill, &m_data_id, sizeof m_data_id);
    p = Create<Packet> (fill, 10);

    return p;
}


} // Namespace ns3
