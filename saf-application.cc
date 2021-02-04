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
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "saf-application.h"


#include "message.h"

/*

 This is a modified version that is the beginings of experimenting to implement
 SAF
 */
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SafApplication");

NS_OBJECT_ENSURE_REGISTERED (SafApplication);

TypeId
SafApplication::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SafApplication")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<SafApplication> ()
    .AddAttribute ("MaxPackets",
                   "The maximum number of packets the application will send",
                   UintegerValue (100),
                   MakeUintegerAccessor (&SafApplication::m_count),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Interval",
                   "The time to wait between packets",
                   TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&SafApplication::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("Port",
                   "The application port",
                   UintegerValue (5000),
                   MakeUintegerAccessor (&SafApplication::m_port),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("StorageSpace",
                "The number of data items the node can hold",
                UintegerValue (15),
                MakeUintegerAccessor (&SafApplication::m_storage_space),
                MakeUintegerChecker<uint16_t> ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&SafApplication::m_txTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("Rx", "A packet has been received",
                     MakeTraceSourceAccessor (&SafApplication::m_rxTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("TxWithAddresses", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&SafApplication::m_txTraceWithAddresses),
                     "ns3::Packet::TwoAddressTracedCallback")
    .AddTraceSource ("RxWithAddresses", "A packet has been received",
                     MakeTraceSourceAccessor (&SafApplication::m_rxTraceWithAddresses),
                     "ns3::Packet::TwoAddressTracedCallback")
  ;
  return tid;
}

SafApplication::SafApplication ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_socket = 0;
  m_sendEvent = EventId ();
  m_data = 0;
  m_port = 5000;
  m_dataSize = 0;

  m_storage_space = 0;
  m_data_items = new Data[m_storage_space];
}

SafApplication::~SafApplication()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_port = 0;

  delete [] m_data;
  delete [] m_data_items;
  m_storage_space = 0;
  m_data = 0;
  m_dataSize = 0;
}

void
SafApplication::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
SafApplication::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);

      if (m_socket->Bind () == -1) {
        NS_FATAL_ERROR ("Failed to bind socket");
      }

      m_socket->Connect (InetSocketAddress (Ipv4Address::GetBroadcast(), m_port));
    }

  //uint8_t fill[] = { 0, 1, 2, 3, 4, 5, 6};
  //SetFill (fill, sizeof(fill), 1024);

  m_socket->SetRecvCallback (MakeCallback (&SafApplication::HandleRead, this));
  m_socket->SetAllowBroadcast (true);
  m_socket->SetIpTtl(2);  // or should this be 0? to only send to 1 hop peers
  ScheduleTransmit (Seconds (0.));
}

void
SafApplication::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0)
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      m_socket = 0;
    }

  Simulator::Cancel (m_sendEvent);
}


// do I care about this?
uint32_t
SafApplication::GetDataSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_size;
}

// fill with real data from a byte buffer,
// fills the send buffer with multiple full/partial copies of fill if needed
void
SafApplication::SetFill (uint8_t *fill, uint32_t fillSize, uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << fill << fillSize << dataSize);
  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  if (fillSize >= dataSize)
    {
      memcpy (m_data, fill, dataSize);
      m_size = dataSize;
      return;
    }

  // Do all but the final fill.
  uint32_t filled = 0;
  while (filled + fillSize < dataSize)
    {
      memcpy (&m_data[filled], fill, fillSize);
      filled += fillSize;
    }

  // Last fill may be partial
  memcpy (&m_data[filled], fill, dataSize - filled);

  // Overwrite packet size attribute.
  m_size = dataSize;
}

// schedule a data send event
void
SafApplication::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  m_sendEvent = Simulator::Schedule (dt, &SafApplication::Send, this);
}

void
SafApplication::Send (void)
{
  NS_LOG_FUNCTION (this);

  // will happen if the event gets canceled
  NS_ASSERT (m_sendEvent.IsExpired ());

  // create the packet to send, will always ensure there is content to send
  Ptr<Packet> p;

  //int8_t fill[] = { m_sent, 1, 2, 3, 4, 5, 6};
  p = Create<Packet> (m_data, m_size);


  Address localAddress;
  m_socket->GetSockName (localAddress);


  // call to the trace sinks before the packet is actually sent,
  // so that tags added to the packet can be sent as well
  m_txTrace (p);
  m_txTraceWithAddresses (p, localAddress, InetSocketAddress (Ipv4Address::GetBroadcast(), m_port));

  m_socket->Send (p);
  m_sent++;

  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
                Ipv4Address::GetBroadcast () << " port " << m_port);


  if (m_sent < m_count) {
    ScheduleTransmit (m_interval);
  }
}

void
SafApplication::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  Ptr<Packet> packet;
  Address from;
  Address localAddress;

  while ((packet = socket->RecvFrom (from)))
    {
        NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client received " << packet->GetSize () << " bytes from " <<
                     InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                     InetSocketAddress::ConvertFrom (from).GetPort ());


      socket->GetSockName (localAddress);
      m_rxTrace (packet);
      m_rxTraceWithAddresses (packet, from, localAddress);
    }
}

// ---------------------------------------------------------------
// ---------------------------------------------------------------

void
SafApplication::LookupData(uint16_t dataID) {

    Data data;
    bool found = false;
    for (int i = 0; i < m_storage_space; i++) {
        data = m_data_items[i];

        if (data.GetDataID() == dataID) {
            data.AccessData(); // increase access frequency
            found = true;
        
            NS_LOG_LOGIC("TODO: Mark cache hit, mark lookup success");
            // mark cache hit, mark successfull lookup
            break;
        }
    }

    if (!found) {
        // send broadcast asking for the data item
        AskPeers(dataID);
    }
}



void
SafApplication::AskPeers(uint16_t dataID) {

  LookupMessage m = LookupMessage(dataID);
  Ptr<Packet> p = m.ToPacket();
  
  Address localAddress;
  m_socket->GetSockName (localAddress);


  // call to the trace sinks before the packet is actually sent,
  // so that tags added to the packet can be sent as well
  m_txTrace (p);
  m_txTraceWithAddresses (p, localAddress, InetSocketAddress (Ipv4Address::GetBroadcast(), m_port));

  m_socket->Send (p);
  m_sent++;

  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s sent request for " << dataID);


}

} // Namespace ns3
