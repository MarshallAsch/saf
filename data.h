


#ifndef SAF_DATA_H
#define SAF_DATA_H

#include "ns3/uinteger.h"
#include <stdio.h>
#include <string.h>


namespace ns3 {

enum class DataStatus {
    unknown,
    free,
    stored,
    pending
};

enum class DataType {
    unkown,
    origianal,
    replica
};


class Data
{
private:
    /* data */
    uint16_t m_data_id;
    uint16_t m_pending_id;
    uint32_t m_size;
    DataStatus m_status;
    DataType m_type;
    //uint16_t m_access_frequency; // how many times this data item is accessed per reloaction period

public:
    Data(); // default do not call this
    Data(uint32_t size);    // use this constructer when creating data items
    Data(uint16_t data_id, uint32_t size); // when saving a replica
    ~Data();
    //void AccessData();
    //void ResetAccessFrequency();
    void SetStatus(DataStatus status);
    uint16_t GetDataID();
    uint16_t GetPendingID();
    uint32_t GetSize();
    //uint16_t GetAccessFrequency();
    DataStatus GetStatus();
};

} //namespace ns3

#endif /*  SAF_DATA_H */
