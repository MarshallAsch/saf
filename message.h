


#ifndef SAF_MESSAGE_H
#define SAF_MESSAGE_H

#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/string.h"

#include <stdio.h>
#include <string.h>

namespace ns3 {

class Message
{
private:
    /* data */
    uint16_t m_request_id;
    double m_requested_at;
    uint16_t m_requested_by;

public:
    Message();
    Message(double seconds, StringValue mac);
    ~Message();
    Packet ToPacket();
};

class LookupMessage:Message 
{
    private:
        uint16_t m_data_id;
    public:
        LookupMessage(uint16_t dataID);
        Ptr<Packet> ToPacket();
};




} //namespace ns3

#endif /*  SAF_DATA_H */
