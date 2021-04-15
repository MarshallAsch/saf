/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "saf-application.h"
#include "ns3/double.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
//#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"

#include <math.h>     // std::pow
#include <algorithm>  // std::sort

#include "logging.h"
#include "message.h"
#include "nsutil.h"
/*

 This is a modified version that is the beginnings of experimenting to implement
 SAF
 */
namespace saf {
using namespace ns3;

NS_OBJECT_ENSURE_REGISTERED(SafApplication);

bool AccessFrequencyComparator(std::vector<uint16_t> i, std::vector<uint16_t> j);

TypeId SafApplication::GetTypeId(void) {
  static TypeId tid =
      TypeId("ns3::SafApplication")
          .SetParent<Application>()
          .SetGroupName("Applications")
          .AddConstructor<SafApplication>()
          .AddAttribute(
              "Port",
              "The application port",
              UintegerValue(5000),
              MakeUintegerAccessor(&SafApplication::m_port),
              MakeUintegerChecker<uint16_t>())
          .AddAttribute(
              "RequestTimeout",
              "The number of seconds util a lookup request times out.",
              TimeValue(10.0_sec),
              MakeTimeAccessor(&SafApplication::m_request_timeout),
              MakeTimeChecker(0.5_sec))
          .AddAttribute(
              "DataSize",
              "The number of bytes in each data object.",
              UintegerValue(30),
              MakeUintegerAccessor(&SafApplication::m_dataSize),
              MakeUintegerChecker<uint32_t>())
          .AddAttribute(
              "ReallocationPeriod",
              "The number of seconds between reallocation events.",
              TimeValue(256.0_sec),
              MakeTimeAccessor(&SafApplication::m_reallocation_period),
              MakeTimeChecker(1.0_sec))
          .AddAttribute(
              "TotalDataItems",
              "The total number of data items in the simulation.",
              UintegerValue(0),
              MakeUintegerAccessor(&SafApplication::m_total_data_items),
              MakeUintegerChecker<uint16_t>())
          .AddAttribute(
              "NumNodes",
              "The total number of nodes in the simulation, used to calculate data items per node.",
              UintegerValue(0),
              MakeUintegerAccessor(&SafApplication::m_total_num_nodes),
              MakeUintegerChecker<uint32_t>())
          .AddAttribute(
              "StorageSpace",
              "The number of data items the node can hold",
              UintegerValue(10),
              MakeUintegerAccessor(&SafApplication::m_replica_space),
              MakeUintegerChecker<uint16_t>())
          .AddAttribute(
              "accessFrequencyMode",
              "The access frequency type, 1, 2, or 3",
              UintegerValue(10),
              MakeUintegerAccessor(&SafApplication::m_access_frequency_type),
              MakeUintegerChecker<uint16_t>())
          .AddAttribute(
              "standardDeviation",
              "the standard deviation for the access frequency calculation",
              DoubleValue(0.0),
              MakeDoubleAccessor(&SafApplication::m_standard_deviation),
              MakeDoubleChecker<double>())
          .AddAttribute(
              "cache_hit_CB",
              "a callback to be called when a data item is looked up but found in the local cache",
              CallbackValue(),
              MakeCallbackAccessor(&SafApplication::m_cache_hit_CB),
              MakeCallbackChecker())
          .AddAttribute(
              "lookup_sent_CB",
              "a callback to be called when the application sends a lookup request",
              CallbackValue(),
              MakeCallbackAccessor(&SafApplication::m_lookup_sent_CB),
              MakeCallbackChecker())
          .AddAttribute(
              "lookup_rcv_CB",
              "a callback to be called when a data item is requested from the nodes peer only "
              "application requests",
              CallbackValue(),
              MakeCallbackAccessor(&SafApplication::m_lookup_rcv_CB),
              MakeCallbackChecker())
          .AddAttribute(
              "lookup_rsp_sent_CB",
              "a callback to be called when a data item being sent to a remote peer only for "
              "application lookups",
              CallbackValue(),
              MakeCallbackAccessor(&SafApplication::m_lookup_rsp_sent_CB),
              MakeCallbackChecker())
          .AddAttribute(
              "lookup_timeout_CB",
              "a callback to be called when the application data lookup request times out",
              CallbackValue(),
              MakeCallbackAccessor(&SafApplication::m_lookup_timeout_CB),
              MakeCallbackChecker())
          .AddAttribute(
              "realloc_timeout_CB",
              "a callback to be called when the reallocation data lookup request times out",
              CallbackValue(),
              MakeCallbackAccessor(&SafApplication::m_realloc_timeout_CB),
              MakeCallbackChecker())
          .AddAttribute(
              "realloc_sent_CB",
              "a callback to be called when the reallocation proccess sends a lookup request",
              CallbackValue(),
              MakeCallbackAccessor(&SafApplication::m_realloc_sent_CB),
              MakeCallbackChecker())
          .AddAttribute(
              "realloc_rcv_CB",
              "a callback to be called when a data item being requested by a remote peer only for "
              "reallocation lookups",
              CallbackValue(),
              MakeCallbackAccessor(&SafApplication::m_realloc_rcv_CB),
              MakeCallbackChecker())
          .AddAttribute(
              "realloc_rsp_sent_CB",
              "a callback to be called when a data item being sent to a remote peer only for "
              "reallocation lookups",
              CallbackValue(),
              MakeCallbackAccessor(&SafApplication::m_realloc_rsp_sent_CB),
              MakeCallbackChecker())
          .AddAttribute(
              "lookup_ontime_CB",
              "a callback to be called when a data item requested by the application is received "
              "before the timeout event",
              CallbackValue(),
              MakeCallbackAccessor(&SafApplication::m_lookup_ontime_CB),
              MakeCallbackChecker())
          .AddAttribute(
              "lookup_late_CB",
              "a callback to be called when a data item requested by the application is received "
              "after the timeout event",
              CallbackValue(),
              MakeCallbackAccessor(&SafApplication::m_lookup_late_CB),
              MakeCallbackChecker())
          .AddAttribute(
              "realloc_ontime_CB",
              "a callback to be called when a data item requested by the reallocation proccess is "
              "received before the timeout event",
              CallbackValue(),
              MakeCallbackAccessor(&SafApplication::m_realloc_ontime_CB),
              MakeCallbackChecker())
          .AddAttribute(
              "realloc_late_CB",
              "a callback to be called when a data item requested by the reallocation proccess is "
              "received after the timeout event",
              CallbackValue(),
              MakeCallbackAccessor(&SafApplication::m_realloc_late_CB),
              MakeCallbackChecker())
          .AddTraceSource(
              "Tx",
              "A new packet is created and is sent",
              MakeTraceSourceAccessor(&SafApplication::m_txTrace),
              "ns3::Packet::TracedCallback")
          .AddTraceSource(
              "Rx",
              "A packet has been received",
              MakeTraceSourceAccessor(&SafApplication::m_rxTrace),
              "ns3::Packet::TracedCallback")
          .AddTraceSource(
              "TxWithAddresses",
              "A new packet is created and is sent",
              MakeTraceSourceAccessor(&SafApplication::m_txTraceWithAddresses),
              "ns3::Packet::TwoAddressTracedCallback")
          .AddTraceSource(
              "RxWithAddresses",
              "A packet has been received",
              MakeTraceSourceAccessor(&SafApplication::m_rxTraceWithAddresses),
              "ns3::Packet::TwoAddressTracedCallback");
  return tid;
}

SafApplication::SafApplication() {
  NS_LOG_FUNCTION(this);
  m_sent = 0;
  m_socket_send = 0;
  m_socket_recv = 0;
  m_running = false;

  m_cache_hit_CB = MakeNullCallback<void, uint16_t, uint32_t>();
  m_lookup_sent_CB = MakeNullCallback<void, uint16_t, uint32_t>();
  m_lookup_rcv_CB = MakeNullCallback<void, uint16_t, uint32_t>();
  m_lookup_rsp_sent_CB = MakeNullCallback<void, uint16_t, uint32_t>();
  m_lookup_timeout_CB = MakeNullCallback<void, uint32_t, uint32_t>();
  m_realloc_timeout_CB = MakeNullCallback<void, uint32_t, uint32_t>();
  m_realloc_sent_CB = MakeNullCallback<void, uint16_t, uint32_t>();
  m_realloc_rcv_CB = MakeNullCallback<void, uint16_t, uint32_t>();
  m_realloc_rsp_sent_CB = MakeNullCallback<void, uint16_t, uint32_t>();

  m_lookup_ontime_CB = MakeNullCallback<void, uint16_t, uint32_t, ns3::Time>();
  m_lookup_late_CB = MakeNullCallback<void, uint16_t, uint32_t, ns3::Time>();
  m_realloc_ontime_CB = MakeNullCallback<void, uint16_t, uint32_t, ns3::Time>();
  m_realloc_late_CB = MakeNullCallback<void, uint16_t, uint32_t, ns3::Time>();
}

SafApplication::~SafApplication() {
  NS_LOG_FUNCTION(this);
  m_socket_send = 0;
  m_socket_recv = 0;
  m_port = 0;

  m_origianal_space = 0;
  m_replica_space = 0;
  m_size = 0;
  m_dataSize = 0;
}

void SafApplication::DoDispose(void) {
  NS_LOG_FUNCTION(this);
  Application::DoDispose();
}

void SafApplication::StartApplication(void) {
  NS_LOG_FUNCTION(this);
  m_running = true;

  if (m_replica_space > m_total_data_items) {
    m_replica_space = m_total_data_items;
  }

  // in the helper there is an assert to ensure this is valid when not in optimized builds
  m_origianal_space = m_total_data_items / m_total_num_nodes;

  // m_origianal_data_items = std::vector<Data>(m_origianal_space);
  // m_replica_data_items = std::vector<Data>(m_replica_space);
  m_access_frequencies = std::vector<std::vector<uint16_t> >(m_total_data_items);

  if (m_socket_recv == 0) {
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    m_socket_recv = Socket::CreateSocket(GetNode(), tid);
    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);

    if (m_socket_recv->Bind(local) == -1) {
      NS_FATAL_ERROR("Failed to bind socket");
    }
  }

  if (m_socket_send == 0) {
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    m_socket_send = Socket::CreateSocket(GetNode(), tid);

    m_socket_send->Connect(InetSocketAddress(Ipv4Address::GetBroadcast(), m_port));
  }
  m_socket_recv->SetRecvCallback(MakeCallback(&SafApplication::HandleRequest, this));
  m_socket_send->SetRecvCallback(MakeCallback(&SafApplication::HandleResponse, this));
  m_socket_send->SetAllowBroadcast(true);
  m_socket_send->SetIpTtl(2);  // or should this be 0? to only send to 1 hop peers

  // fill access frequencies using random values
  // then sort the access frequencies
  GenerateDataItems();

  for (uint16_t i = 1; i <= m_total_data_items; i++) {
    double accessFrequency = CalculateAccessFrequency(i);
    double lookupDelay =
        m_reallocation_period.GetSeconds() - (m_reallocation_period.GetSeconds() * accessFrequency);

    Ptr<ExponentialRandomVariable> e = CreateObject<ExponentialRandomVariable>();
    e->SetAttribute("Mean", DoubleValue(lookupDelay));
    m_data_lookup_generator.push_back(e);
    std::vector<uint16_t> row(2);
    row[0] = i;                   // dataID
    row[1] = lookupDelay * 1000;  // to convert to an int
    m_access_frequencies[i - 1] = row;
  }
  sort(m_access_frequencies.begin(), m_access_frequencies.end(), AccessFrequencyComparator);

  // schedule first reallocation event
  m_reallocation_event =
      Simulator::Schedule(m_reallocation_period, &SafApplication::RunReplication, this);

  // schedule data lookups
  ScheduleFirstLookups();
}

void SafApplication::StopApplication() {
  NS_LOG_FUNCTION(this);
  m_running = false;

  if (m_socket_recv != 0) {
    m_socket_recv->Close();
    m_socket_recv->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> >());
    m_socket_recv = 0;
  }

  if (m_socket_send != 0) {
    m_socket_send->Close();
    m_socket_send->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> >());
    m_socket_send = 0;
  }

  // check to see if it is still in the pending lookup list
  for (std::set<uint32_t>::iterator it = m_pending_lookups.begin(); it != m_pending_lookups.end();
       it++) {
    NS_LOG_INFO("TODO: sim ended before application request ID " << *it << " timed out");
  }

  // check to see if it is still in the pending lookup list
  for (std::set<uint32_t>::iterator it = m_pending_reallocations.begin();
       it != m_pending_reallocations.end();
       it++) {
    NS_LOG_INFO("TODO: sim ended before reallocation request ID " << *it << " timed out");
  }

  Simulator::Cancel(m_reallocation_event);
}

double SafApplication::CalculateAccessFrequency(uint16_t dataID) {
  if (m_access_frequency_type == 1) {
    return 0.5 * (1.0 + 0.01 * dataID);
  } else if (m_access_frequency_type == 2) {
    return 0.025 * dataID;
  } else if (m_access_frequency_type == 3) {
    Ptr<NormalRandomVariable> x = CreateObject<NormalRandomVariable>();
    x->SetAttribute("Mean", DoubleValue(0.5 * (1.0 + 0.01 * dataID)));
    x->SetAttribute("Variance", DoubleValue(pow(m_standard_deviation, 2.0)));

    return x->GetValue();
  } else {
    NS_LOG_ERROR("this is bad, the access frequency mode must be 1, 2, or 3");
    return 0;
  }
}

void SafApplication::ScheduleFirstLookups() {
  NS_LOG_FUNCTION(this);

  for (uint16_t i = 1; i <= m_total_data_items; i++) {
    double dt = m_data_lookup_generator[i - 1]->GetValue();
    Simulator::Schedule(Seconds(dt), &SafApplication::ScheduleNextLookup, this, i);
  }
}

void SafApplication::ScheduleNextLookup(uint16_t dataID) {
  NS_LOG_FUNCTION(this);
  // dont schedule the next event if it is no longer running
  if (!m_running) {
    NS_LOG_INFO("Simulation done, canceling lookup");
    return;
  }

  LookupData(dataID);
  double dt = m_data_lookup_generator[dataID - 1]->GetValue();
  if (Simulator::Now() + Seconds(dt) < m_stopTime) {
    Simulator::Schedule(Seconds(dt), &SafApplication::ScheduleNextLookup, this, dataID);
  }
}

void SafApplication::HandleRequest(Ptr<Socket> socket) {
  NS_LOG_FUNCTION(this << socket);

  Ptr<Packet> packet;
  Address from;
  Address localAddress;

  while ((packet = socket->RecvFrom(from))) {
    socket->GetSockName(localAddress);
    NS_LOG_INFO(
        "At time " << Simulator::Now().GetSeconds() << "s client received " << packet->GetSize()
                   << " bytes from " << InetSocketAddress::ConvertFrom(from).GetIpv4() << " port "
                   << InetSocketAddress::ConvertFrom(from).GetPort());

    m_rxTrace(packet);
    m_rxTraceWithAddresses(packet, from, localAddress);

    uint32_t size = packet->GetSize();
    uint8_t* payload = new uint8_t[size];
    packet->CopyData(payload, size);

    if (payload[0] == uint8_t(MessageType::lookup)) {
      NS_LOG_INFO("RECEIVED lookup command");

      uint32_t requestID;
      uint64_t sentAt;
      uint16_t dataID;
      bool isReplication;
      memcpy(&requestID, &payload[5], sizeof(requestID));
      memcpy(&sentAt, &payload[14], sizeof(sentAt));
      memcpy(&dataID, &payload[30], sizeof(dataID));
      isReplication = payload[32];

      delete[] payload;

      // Keeping this for the eventual debug purpose when I wonder why the data is not working
      // std::cout << "-----------------------\n";
      // std::cout << "sent at: " << sentAt << "\n";
      // std::cout << "req dataID: " << dataID << "\n";

      // std::cout << "receive: ";
      // for (uint32_t i = 30 ; i < 32; i++) {
      //   std::cout << unsigned(payload[i]) << " ";
      // }
      // std::cout << "\n";

      // mark that the lookup request was received, this is to be able to detect collisions
      if (isReplication) {
        if (!m_realloc_rcv_CB.IsNull()) m_realloc_rcv_CB(dataID, GetNode()->GetId());
      } else {
        if (!m_lookup_rcv_CB.IsNull()) m_lookup_rcv_CB(dataID, GetNode()->GetId());
      }

      Data item = GetDataItem(dataID);
      if (item.GetStatus() != DataStatus::stored) {
        NS_LOG_INFO("Data item not found, not sending response");
        continue;
      }

      NS_LOG_INFO("sending response");
      // generate and send response
      ResponseMessage r = ResponseMessage(requestID, sentAt, isReplication, item);
      Ptr<Packet> responsePacket = r.ToPacket();

      if (isReplication) {
        if (!m_realloc_rsp_sent_CB.IsNull()) m_realloc_rsp_sent_CB(dataID, GetNode()->GetId());
      } else {
        if (!m_lookup_rsp_sent_CB.IsNull()) m_lookup_rsp_sent_CB(dataID, GetNode()->GetId());
      }

      socket->SendTo(responsePacket, 0, from);
      NS_LOG_INFO("sent packet");
    }
  }
}

void SafApplication::HandleResponse(Ptr<Socket> socket) {
  NS_LOG_FUNCTION(this << socket);

  Ptr<Packet> packet;
  Address from;
  Address localAddress;

  while ((packet = socket->RecvFrom(from))) {
    socket->GetSockName(localAddress);
    NS_LOG_INFO(
        "At time " << Simulator::Now().GetSeconds() << "s client received response "
                   << packet->GetSize() << " bytes from "
                   << InetSocketAddress::ConvertFrom(from).GetIpv4() << " port "
                   << InetSocketAddress::ConvertFrom(from).GetPort());

    m_rxTrace(packet);
    m_rxTraceWithAddresses(packet, from, localAddress);

    uint32_t size = packet->GetSize();
    uint8_t* payload = new uint8_t[size];
    packet->CopyData(payload, size);

    NS_LOG_INFO("handling data received: " << payload[0]);

    if (payload[0] == uint8_t(MessageType::dataResponse)) {
      NS_LOG_INFO("handling data received");
      uint16_t dataID;
      uint32_t dataSize;
      uint32_t origID;
      uint64_t askTime;
      bool isReplication;

      memcpy(&dataSize, &payload[1], sizeof(dataSize));
      memcpy(&askTime, &payload[22], sizeof(askTime));
      memcpy(&origID, &payload[9], sizeof(origID));
      memcpy(&dataID, &payload[30], sizeof(dataID));
      isReplication = payload[32];
      dataSize -= (sizeof(dataID) - 1);
      delete[] payload;

      Data item = Data(dataID, dataSize);
      SaveDataItem(item);

      // remove from pending request list
      std::set<uint32_t>::iterator it;
      Time diff = Simulator::Now() - Time::FromInteger(askTime, Time::Unit::MS);

      if (isReplication) {
        it = m_pending_reallocations.find(origID);
        if (it != m_pending_reallocations.end()) {
          m_pending_reallocations.erase(it);
          if (!m_realloc_ontime_CB.IsNull()) m_realloc_ontime_CB(dataID, GetNode()->GetId(), diff);
          // log successful request
        } else {
          // log successful request, already gotten or late
          if (!m_realloc_late_CB.IsNull()) m_realloc_late_CB(dataID, GetNode()->GetId(), diff);
        }
      } else {
        it = m_pending_lookups.find(origID);
        if (it != m_pending_lookups.end()) {
          m_pending_lookups.erase(it);
          if (!m_lookup_ontime_CB.IsNull()) m_lookup_ontime_CB(dataID, GetNode()->GetId(), diff);
          // log successful request
        } else {
          // log successful request, already gotten or late
          if (!m_lookup_late_CB.IsNull()) m_lookup_late_CB(dataID, GetNode()->GetId(), diff);
        }
      }

      NS_LOG_LOGIC("TODO: Mark cache miss, mark lookup success, remove from pending reponse list");
    }
  }
}

// ---------------------------------------------------------------
// ---------------------------------------------------------------

void SafApplication::GenerateDataItems() {
  NS_LOG_FUNCTION(this);
  for (int i = 0; i < m_origianal_space; i++) {
    m_origianal_data_items.push_back(Data(m_dataSize));
  }
}

void SafApplication::LookupData(uint16_t dataID) {
  NS_LOG_FUNCTION(this);
  Data item = GetDataItem(dataID);

  if (item.GetStatus() == DataStatus::stored) {
    if (!m_cache_hit_CB.IsNull()) m_cache_hit_CB(dataID, GetNode()->GetId());
  } else {
    // send broadcast asking for the data item
    AskPeers(dataID, false);
  }
}

void SafApplication::SaveDataItem(Data data) {
  NS_LOG_FUNCTION(this);

  bool found = false;
  bool stored = false;
  for (std::vector<Data>::iterator it = m_replica_data_items.begin();
       it != m_replica_data_items.end();
       ++it) {
    if (!found && m_access_frequencies[it - m_replica_data_items.begin()][0] == data.GetDataID()) {
      found = true;
    }

    if (!stored && (*it).GetDataID() == data.GetDataID()) {
      stored = true;
    }
  }
  if (!found || stored) {
    NS_LOG_INFO("data: " << data.GetDataID() << " Is not being saved");
  }

  if (m_replica_data_items.size() < m_replica_space) {
    m_replica_data_items.push_back(data);
  }
}

Data SafApplication::GetDataItem(uint16_t dataID) {
  NS_LOG_FUNCTION(this);
  for (int i = 0; i < m_origianal_space; i++) {
    if (m_origianal_data_items[i].GetDataID() == dataID) {
      return m_origianal_data_items[i];
    }
  }

  for (std::vector<Data>::iterator it = m_replica_data_items.begin();
       it != m_replica_data_items.end();
       ++it) {
    if ((*it).GetDataID() == dataID) {
      return (*it);
    }
  }

  return Data();
}

void SafApplication::AskPeers(uint16_t dataID, bool isReplication) {
  NS_LOG_FUNCTION(this);
  LookupMessage m = LookupMessage(dataID, isReplication);
  Ptr<Packet> p = m.ToPacket();

  Address localAddress;
  m_socket_send->GetSockName(localAddress);

  // call to the trace sinks before the packet is actually sent,
  // so that tags added to the packet can be sent as well
  m_txTrace(p);
  m_txTraceWithAddresses(p, localAddress, InetSocketAddress(Ipv4Address::GetBroadcast(), m_port));

  // TODO: use add a hook to the router to get all of the other one hop nodes in the routing table
  // to get the total
  // number of recipients

  if (isReplication) {
    m_pending_reallocations.insert(m.getRequestID());  // add to pending list
    // stats for reallocation
    if (!m_realloc_sent_CB.IsNull()) m_realloc_sent_CB(dataID, GetNode()->GetId());

    if (Simulator::Now() + m_request_timeout < m_stopTime) {
      Simulator::
          Schedule(m_request_timeout, &SafApplication::ReallocationTimeout, this, m.getRequestID());
    }
  } else {
    m_pending_lookups.insert(m.getRequestID());  // add to pending list
    // stats for 'normal lookup'
    if (!m_lookup_sent_CB.IsNull()) m_lookup_sent_CB(dataID, GetNode()->GetId());

    if (Simulator::Now() + m_request_timeout < m_stopTime) {
      Simulator::
          Schedule(m_request_timeout, &SafApplication::LookupTimeout, this, m.getRequestID());
    }
  }

  m_socket_send->Send(p);
  m_sent++;

  NS_LOG_INFO("At time " << Simulator::Now().GetSeconds() << "s sent request for " << dataID);
}

void SafApplication::LookupTimeout(uint32_t requestID) {
  NS_LOG_FUNCTION(this);

  // check to see if it is still in the pending lookup list
  std::set<uint32_t>::iterator item = m_pending_lookups.find(requestID);

  if (item != m_pending_lookups.end()) {
    if (!m_lookup_timeout_CB.IsNull()) m_lookup_timeout_CB(requestID, GetNode()->GetId());
    m_pending_lookups.erase(item);
  }
}

void SafApplication::ReallocationTimeout(uint32_t requestID) {
  NS_LOG_FUNCTION(this);

  // check to see if it is still in the pending lookup list
  std::set<uint32_t>::iterator item = m_pending_reallocations.find(requestID);

  if (item != m_pending_reallocations.end()) {
    if (!m_realloc_timeout_CB.IsNull()) m_realloc_timeout_CB(requestID, GetNode()->GetId());
    m_pending_reallocations.erase(item);
  }
}

void SafApplication::RunReplication() {
  NS_LOG_FUNCTION(this);

  // check if all the items are stored
  if (m_replica_data_items.size() == m_replica_space) {
    return;
  }

  // check to see which items are not yet found, and request them if necessary
  for (uint16_t i = 0; i < m_replica_space; i++) {
    bool found = false;
    for (std::vector<Data>::iterator it = m_replica_data_items.begin();
         it != m_replica_data_items.end();
         ++it) {
      if ((*it).GetStatus() == DataStatus::stored &&
          (*it).GetDataID() == m_access_frequencies[i][0]) {
        found = true;
        break;
      }
    }

    if (!found) AskPeers(m_access_frequencies[i][0], true);
  }

  // schedule next reallocation event
  m_reallocation_event =
      Simulator::Schedule(m_reallocation_period, &SafApplication::RunReplication, this);
}

// compariator for sorting access frequencies sorts highest to lowest
bool AccessFrequencyComparator(std::vector<uint16_t> i, std::vector<uint16_t> j) {
  return i[1] > j[1];
}

}  // Namespace saf
