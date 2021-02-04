


#ifndef SAF_DATA_H
#define SAF_DATA_H

#include "ns3/uinteger.h"
#include <stdio.h>
#include <string.h>


namespace ns3 {

enum DataStatus {
    unknown = 0,
    free = 1,
    stored = 2,
    pending = 4
};

class Data
{
private:
    /* data */
    uint16_t m_data_id;
    uint16_t m_size;
    uint8_t *m_data;
    uint8_t m_status;
    uint16_t m_access_frequency; // how many times this data item is accessed per reloaction period

public:
    Data();
    Data(uint16_t data_id, uint8_t *payload, uint16_t size);
    ~Data();
    void AccessData();
    void ResetAccessFrequency();
    void SetStatus(uint8_t status);
    uint16_t GetDataID();
    uint16_t GetAccessFrequency();
    uint8_t getStatus();

};

} //namespace ns3

#endif /*  SAF_DATA_H */
