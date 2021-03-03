
#include "data.h"

namespace saf {
using namespace ns3;

// default when creating the lists
Data::Data() {
  m_data_id = 0;
  m_pending_id = 0;
  m_size = 0;
  // m_access_frequency = 0;
  m_status = DataStatus::free;
  m_type = DataType::unkown;
}

// when generating a new data item
Data::Data(uint32_t size) {
  static uint16_t id = 1;
  m_data_id = id++;
  m_pending_id = 0;
  m_size = size;
  m_status = DataStatus::stored;
  m_type = DataType::origianal;
}

// when saving a data item
Data::Data(uint16_t data_id, uint32_t size) {
  m_data_id = data_id;
  m_pending_id = 0;
  m_size = size;
  m_status = DataStatus::stored;
  m_type = DataType::replica;
}

Data::~Data() {
  m_data_id = 0;
  m_pending_id = 0;
  m_size = 0;
  m_status = DataStatus::unknown;
  m_type = DataType::unkown;
}

void Data::SetStatus(DataStatus status) { m_status = status; }

uint16_t Data::GetDataID() { return m_data_id; }

uint16_t Data::GetPendingID() { return m_pending_id; }

uint32_t Data::GetSize() { return m_size; }

DataStatus Data::GetStatus() { return m_status; }

}  // Namespace saf
