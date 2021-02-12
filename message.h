


#ifndef SAF_MESSAGE_H
#define SAF_MESSAGE_H

#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/string.h"

#include <stdio.h>
#include <string.h>
#include <vector>

#include "data.h"

namespace ns3 {

enum class MessageType: uint8_t {
    unknown = 0x00,
    lookup = 0x01,
    dataResponse = 0x02
};

class Message
{
    private:
        virtual std::vector<uint8_t> GeneratePayload();
    protected:
        /* data */
        uint32_t m_request_id;
        uint32_t m_response_id;
        int64_t m_requested_at;
        MessageType m_type;

    public:
        Message();
        ~Message();
        Ptr<Packet> ToPacket();
};

class LookupMessage:public Message
{
    private:
        uint16_t m_data_id;
        std::vector<uint8_t> GeneratePayload();
    public:
        LookupMessage(uint16_t dataID);

};

class ResponseMessage:public Message
{
    private:
        uint16_t m_data_id; // the data object this is a response for
        // dont need to actually have the data conent here
        uint32_t m_data_size;
        std::vector<uint8_t> GeneratePayload();
    public:
        ResponseMessage(uint32_t requestID, Data data);
};


} //namespace ns3

#endif /*  SAF_DATA_H */
