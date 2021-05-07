/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef SAF_H
#define SAF_H

#include <set>     // std::set
#include <vector>  // std::vector

#include "ns3/application.h"
#include "ns3/basic-data-calculators.h"
#include "ns3/callback.h"
#include "ns3/data-collector.h"
#include "ns3/event-id.h"
#include "ns3/ipv4-address.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include "ns3/socket.h"
#include "ns3/time-data-calculators.h"
#include "ns3/traced-callback.h"

#include "data.h"

namespace ns3 {

/**
 * \ingroup udpecho
 * \brief A Udp Echo client
 *
 * Every packet sent should be returned by the server and received here.
 */
class SafApplication : public Application {
 public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId(void);

  SafApplication();

  virtual ~SafApplication();

  /**
   * Get the number of data bytes that will be sent to the server.
   *
   * \warning The number of bytes may be modified by calling any one of the
   * SetFill methods.  If you have called SetFill, then the number of
   * data bytes will correspond to the size of an initialized data buffer.
   * If you have not called a SetFill method, the number of data bytes will
   * correspond to the number of don't care bytes that will be sent.
   *
   * \returns The number of data bytes.
   */
  uint32_t GetDataSize(void) const;

  /**
   * Set the data fill of the packet (what is sent as data to the server) to
   * the contents of the fill buffer, repeated as many times as is required.
   *
   * Initializing the packet to the contents of a provided single buffer is
   * accomplished by setting the fillSize set to your desired dataSize
   * (and providing an appropriate buffer).
   *
   * \warning The size of resulting echo packets will be automatically adjusted
   * to reflect the dataSize parameter -- this means that the PacketSize
   * attribute of the Application may be changed as a result of this call.
   *
   * \param fill The fill pattern to use when constructing packets.
   * \param fillSize The number of bytes in the provided fill pattern.
   * \param dataSize The desired size of the final echo data.
   */
  void SetFill(uint8_t* fill, uint32_t fillSize, uint32_t dataSize);

 protected:
  virtual void DoDispose(void);

 private:
  virtual void StartApplication(void);
  virtual void StopApplication(void);

  /**
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param socket the socket the packet was received to.
   */
  void HandleRequest(Ptr<Socket> socket);

  void HandleResponse(Ptr<Socket> socket);

  void GenerateDataItems();

  void SaveDataItem(Data data);

  void AskPeers(uint16_t dataID, bool isReplication);

  void LookupData(uint16_t dataID);

  static uint32_t GenMessageID();

  uint32_t m_size;  //!< Size of the sent packet

  uint32_t m_dataSize;  //!< packet payload size (must be equal to m_size)

  uint32_t m_sent;            //!< Counter for sent packets
  Ptr<Socket> m_socket_send;  //!< Socket
  Ptr<Socket> m_socket_recv;  //!< Socket

  uint16_t m_port;  //!< Remote peer port

  EventId m_reallocation_event;  // for pending reallocation events

  std::vector<Data> m_replica_data_items;    // the block of memory to hold the data items
  std::vector<Data> m_origianal_data_items;  // the block of memory to hold the
                                             // originals data items

  std::vector<std::vector<uint16_t>> m_access_frequencies;
  std::set<uint32_t> m_pending_lookups;
  std::set<uint32_t> m_pending_reallocations;

  // uint16_t* m_access_frequencies; // since the access frequencies are static
  // and known for all data items
  uint16_t m_total_data_items;
  uint32_t m_total_num_nodes;

  uint16_t m_origianal_space;  // the number of data items that can be stored by
                               // the node
  uint16_t m_replica_space;    // the number of data items that can be stored by
                               // the node

  uint16_t m_access_frequency_type;
  double m_standard_deviation;

  ns3::Time m_request_timeout;
  ns3::Time m_reallocation_period;

  bool m_running;

  std::vector<Ptr<ExponentialRandomVariable>> m_data_lookup_generator;

  double CalculateAccessFrequency(uint16_t dataID);

  Data GetDataItem(uint16_t dataID);

  void LookupTimeout(uint32_t requestID);

  void ReallocationTimeout(uint32_t requestID);

  void RunReplication();

  void ScheduleFirstLookups();

  void ScheduleNextLookup(uint16_t dataID);

  /// Callbacks for tracing the packet Tx events
  TracedCallback<Ptr<const Packet>> m_txTrace;

  /// Callbacks for tracing the packet Rx events
  TracedCallback<Ptr<const Packet>> m_rxTrace;

  /// Callbacks for tracing the packet Tx events, includes source and
  /// destination addresses
  TracedCallback<Ptr<const Packet>, const Address&, const Address&> m_txTraceWithAddresses;

  /// Callbacks for tracing the packet Rx events, includes source and
  /// destination addresses
  TracedCallback<Ptr<const Packet>, const Address&, const Address&> m_rxTraceWithAddresses;

  Callback<void, uint16_t, uint32_t> m_cache_hit_CB;
  Callback<void, uint16_t, uint32_t> m_lookup_sent_CB;
  Callback<void, uint16_t, uint32_t> m_lookup_rcv_CB;
  Callback<void, uint16_t, uint32_t> m_lookup_rsp_sent_CB;
  Callback<void, uint32_t, uint32_t> m_lookup_timeout_CB;
  Callback<void, uint32_t, uint32_t> m_realloc_timeout_CB;
  Callback<void, uint16_t, uint32_t> m_realloc_sent_CB;
  Callback<void, uint16_t, uint32_t> m_realloc_rcv_CB;
  Callback<void, uint16_t, uint32_t> m_realloc_rsp_sent_CB;

  Callback<void, uint16_t, uint32_t, ns3::Time> m_lookup_ontime_CB;
  Callback<void, uint16_t, uint32_t, ns3::Time> m_lookup_late_CB;
  Callback<void, uint16_t, uint32_t, ns3::Time> m_realloc_ontime_CB;
  Callback<void, uint16_t, uint32_t, ns3::Time> m_realloc_late_CB;
};
}  // namespace ns3

#endif /* SAF_H */
