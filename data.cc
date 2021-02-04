
#include "data.h"


namespace ns3 {

Data::Data() 
{
    m_size = 0;
    m_data = NULL;
    m_data_id = 0;
    m_access_frequency = 0;
}


Data::Data(uint16_t data_id, uint8_t *payload, uint16_t size)
{
    m_data = new uint8_t[size];
    memcpy(m_data, payload, size);
    m_data_id = data_id;
    m_size = size;
    m_access_frequency = 0;
    m_status = 0;
}

Data::~Data()
{
    delete[] m_data;
    m_data = 0;
    m_data_id = 0;
    m_size = 0;
    m_access_frequency = 0;
    m_status = 0;
}

void 
Data::AccessData()
{
    m_access_frequency++;
}

void
Data::ResetAccessFrequency()
{
    m_access_frequency = 0;
}

void
Data::SetStatus(uint8_t status)
{
    m_status = status;
}

uint16_t
Data::GetDataID()
{
    return m_data_id;
}

uint16_t
Data::GetAccessFrequency()
{
    return m_access_frequency;
}

uint8_t
Data::getStatus()
{
    return m_status;
}

} // Namespace ns3
