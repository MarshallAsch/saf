/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

/* This script configures two nodes connected vis CSMA channel. One of the
 *  nodes transmits packets via PPBP application to the other node.
 *  Command line parameters are the simulationTime and verbose for logging.
 *  Example run: Copy to scratch folder and run
 *  ./waf --run "scratch/PPBP-application-test --simulationTime=10.0 --verbose=true"
 *  Author: Sharan Naribole <nsharan@rice.edu>
 */

/**
 *
 * This is the third test of a basic NS3 simulation that will create a one goup of nodes
 * and have them move arround randomly. This will also create an animation for netanim
 *
 * This is the first part in the progressive tesing.
 * This is a continuation of part 1 where we setup a wireless mesh network for the nodes.
 * For the setup of the wireless parameters we have given some justification for why certain
 * default options and parameters were selected.
 *
 * The next step for part 3 will be to create a simple application to generate traffic in the
 * network.
 *
 * The following step for part 4 will be to add the ability to collect statistics about various
 * parts of the simulation.
 */

#include "ns3/application-container.h"
#include "ns3/applications-module.h"
#include "ns3/attribute.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/omnet-data-output.h"
#include "ns3/sqlite-data-output.h"

#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

#include "ns3/aodv-helper.h"
#include "ns3/wifi-module.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-helper.h"

#include "ns3/saf-helper.h"
#include "ns3/util.h"

#include "logging.h"
#include "nsutil.h"
#include "simulation-params.h"


using namespace ns3;
using namespace saf;

bool verbose = true;

DataCollector data;

Ptr<CounterCalculator<> > m_cache_hit;

// these is only for the application level data lookups
// this will exclude the lookups for reallocation purposes
Ptr<CounterCalculator<> > m_lookup_sent;
Ptr<CounterCalculator<> > m_lookup_rcv;

Ptr<CounterCalculator<> > m_lookup_rsp_sent;
// lookup rsp receive will bt the lookup late + lookup ontime

// these will count the number of times that the request timed out
// of times late is high then the application timeout should be increased
// if the timouts is higher then the late then it probably means that data is lost
// or that no one has what your looking for and a longer timeout will not improve
// things
Ptr<CounterCalculator<> > m_lookup_timeout;
Ptr<CounterCalculator<> > m_realloc_timeout;

// this will count the lookups for just the reallocation
// lookups  not the application ones
Ptr<CounterCalculator<> > m_realloc_sent;
Ptr<CounterCalculator<> > m_realloc_rcv;

Ptr<CounterCalculator<> > m_realloc_rsp_sent;
// realoc rsp receive will bt the realoc late + realoc ontime

// timings for application lookup responses
Ptr<TimeMinMaxAvgTotalCalculator> m_lookup_ontime;
Ptr<TimeMinMaxAvgTotalCalculator> m_lookup_late;

// timings for realloc responses
Ptr<TimeMinMaxAvgTotalCalculator> m_realloc_ontime;
Ptr<TimeMinMaxAvgTotalCalculator> m_realloc_late;

void cache_hit_CB(uint16_t dataID, uint32_t nodeID) { m_cache_hit->Update(); }

void lookup_sent_CB(uint16_t dataID, uint32_t nodeID) { m_lookup_sent->Update(); }

void lookup_rcv_CB(uint16_t dataID, uint32_t nodeID) { m_lookup_rcv->Update(); }

void lookup_rsp_sent_CB(uint16_t dataID, uint32_t nodeID) { m_lookup_rsp_sent->Update(); }

void lookup_timeout_CB(uint32_t requestID, uint32_t nodeID) { m_lookup_timeout->Update(); }

void realloc_timeout_CB(uint32_t requestID, uint32_t nodeID) { m_realloc_timeout->Update(); }

void realloc_sent_CB(uint16_t dataID, uint32_t nodeID) { m_realloc_sent->Update(); }

void realloc_rcv_CB(uint16_t dataID, uint32_t nodeID) { m_realloc_rcv->Update(); }

void realloc_rsp_sent_CB(uint16_t dataID, uint32_t nodeID) { m_realloc_rsp_sent->Update(); }

void lookup_ontime_CB(uint16_t dataID, uint32_t nodeID, Time delay) {
  m_lookup_ontime->Update(delay);
}

void lookup_late_CB(uint16_t dataID, uint32_t nodeID, Time delay) { m_lookup_late->Update(delay); }

void realloc_ontime_CB(uint16_t dataID, uint32_t nodeID, Time delay) {
  m_realloc_ontime->Update(delay);
}

void realloc_late_CB(uint16_t dataID, uint32_t nodeID, Time delay) {
  m_realloc_late->Update(delay);
}

void setupStats(uint32_t runNum, std::string input) {
  // change some of this stuff to real values that are not hardcoded
  data.DescribeRun("SAF experiment", "wireless", input, std::to_string(runNum));
  data.AddMetadata("Author", "Marshall Asch");

  m_cache_hit = CreateObject<CounterCalculator<> >();
  m_lookup_sent = CreateObject<CounterCalculator<> >();
  m_lookup_rcv = CreateObject<CounterCalculator<> >();
  m_lookup_rsp_sent = CreateObject<CounterCalculator<> >();
  m_lookup_timeout = CreateObject<CounterCalculator<> >();
  m_realloc_timeout = CreateObject<CounterCalculator<> >();
  m_realloc_sent = CreateObject<CounterCalculator<> >();
  m_realloc_rcv = CreateObject<CounterCalculator<> >();
  m_realloc_rsp_sent = CreateObject<CounterCalculator<> >();

  m_lookup_ontime = CreateObject<TimeMinMaxAvgTotalCalculator>();
  m_lookup_late = CreateObject<TimeMinMaxAvgTotalCalculator>();
  m_realloc_ontime = CreateObject<TimeMinMaxAvgTotalCalculator>();
  m_realloc_late = CreateObject<TimeMinMaxAvgTotalCalculator>();

  m_cache_hit->SetKey("cache-hit");
  m_lookup_sent->SetKey("lookup-sent");
  m_lookup_rcv->SetKey("lookup-rcv");
  m_lookup_rsp_sent->SetKey("lookup-rsp-sent");
  m_lookup_timeout->SetKey("lookup-timeout");
  m_realloc_timeout->SetKey("realloc-timeout");
  m_realloc_sent->SetKey("realloc-sent");
  m_realloc_rcv->SetKey("realloc-rcv");
  m_realloc_rsp_sent->SetKey("realloc-rsp-sent");
  m_lookup_ontime->SetKey("lookup-ontime-delay");
  m_lookup_late->SetKey("lookup-late-delay");
  m_realloc_ontime->SetKey("realloc-ontime-delay");
  m_realloc_late->SetKey("realloc-late-delay");

  data.AddDataCalculator(m_cache_hit);
  data.AddDataCalculator(m_lookup_sent);
  data.AddDataCalculator(m_lookup_rcv);
  data.AddDataCalculator(m_lookup_rsp_sent);
  data.AddDataCalculator(m_lookup_timeout);
  data.AddDataCalculator(m_realloc_timeout);
  data.AddDataCalculator(m_realloc_sent);
  data.AddDataCalculator(m_realloc_rcv);
  data.AddDataCalculator(m_realloc_rsp_sent);
  data.AddDataCalculator(m_lookup_ontime);
  data.AddDataCalculator(m_lookup_late);
  data.AddDataCalculator(m_realloc_ontime);
  data.AddDataCalculator(m_realloc_late);
}

/**
 * This is the main entry point to start the simulator.
 * Needs to set everything up so that it can run
 *
 */
int main(int argc, char* argv[]) {
  // application parameters to be added
  // num nodes
  // num data items (must be less than 100 data items )
  // memory space - C (10, 1-39)
  // data item size
  // data access rates for each item is known and constant
  // relocation period - T (unused for SAF) (256, 1-8192)
  // max node speed - d (1)
  // communication range - R (7, 1-19)
  // simulation time = 50000

  SimulationParameters params;
  bool ok;
  std::tie(params, ok) = SimulationParameters::parse(argc, argv);

  Time::SetResolution(Time::NS);

  if (!ok) {
    std::cerr << "Error parsing the parameters.\n";
    return -1;
  }

  if (params.dryRun) {
    std::cout << "dry run only printing the paramaters this is going to be run with\n";
    std::cout << "==============================================================\n";
    std::cout << std::string(params) << "\n";
    return 0;
  }

  // this will set a seed so that the same numbers are not generated each time.
  // the run number should be incremented each time this simulation is run to ensure streams do not
  // overlap
  RngSeedManager::SetSeed(params.seed);
  RngSeedManager::SetRun(params.runNumber);

  setupStats(params.runNumber, std::string(params));

  // create node containers
  NodeContainer nodes;
  nodes.Create(params.totalNodes);

  // create mobility model
  MobilityHelper mobility;
  mobility.SetMobilityModel(
      "ns3::RandomWaypointMobilityModel",
      "Speed",
      PointerValue(params.speed),
      "Pause",
      PointerValue(params.pause),
      "PositionAllocator",
      PointerValue(params.positionAllocator));
  mobility.SetPositionAllocator(PointerValue(params.positionAllocator));

  mobility.Install(nodes);

  // create the wifi ad hoc network interfaces.

  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
  wifiPhy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

  // Rx and Tx gain should be 0, because apparenlty thats a rule for mobile devices from the FCC?
  // maybe just a small num between 0 and 5?
  // leaving the default rx sensitivity of -101dB

  // for the physical propagation loss model consider using a chain of loss
  // models to account for different properties
  // ChannelConditionModel - dynamicly change model if there is LOS or not if using buildings
  // friisPropagationLossModel - distance propagation?
  // -- assumes free space not great choice
  // COST hata model
  // -- urban area - requires specifying the antenna locations
  // log distance path loss
  // -- suburban, probably this one

  // delay propagation, constant speed, note this assumes the earth is flat, still use it
  // the simulation areas are small enough that it should be fine.

  YansWifiChannelHelper wifiChannel;

  wifiChannel = YansWifiChannelHelper::Default();
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");

  // I found a paper that measured values for the exponent and reference loss
  // exponent = between < 2 and > 6, depending on environment, open hallway and crossing walls
  // and floors respectively since this simulation is assuming a city/town setting we will select
  // a value of n of 3 to account for mostly open outdoor space with some obsticles reference loss
  // for the 2.4 GHz is around 41.7 dB apps.dtic.mil/dtic/tr/fulltext/u2/a25656s.pdf (page 19)
  // wifiChannel.AddPropagationLoss(
  //     "ns3::LogDistancePropagationLossModel",
  //     "Exponent",
  //     DoubleValue(3),
  //     "ReferenceDistance",
  //     DoubleValue(1.0),
  //     "ReferenceLoss",
  //     DoubleValue(41.7));
  wifiPhy.SetChannel(wifiChannel.Create());

  wifiChannel.AddPropagationLoss(
      "ns3::RangePropagationLossModel",
      "MaxRange",
      DoubleValue(params.wifiRadius));
  wifiPhy.SetChannel(wifiChannel.Create());

  // set radio to ad hoc network mode? This seems to be needed
  // this seems to be fine to leave this with all of its default parameters
  WifiMacHelper wifiMac;
  wifiMac.SetType("ns3::AdhocWifiMac");

  WifiHelper wifi;
  // 802.11b was selected as the physical channel because almost all consumer devices support it
  // and it has lower power requirements and larger transmission ranges then other wifi standards
  wifi.SetStandard(WIFI_STANDARD_80211b);

  NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, nodes);

  InternetStackHelper internet;

  if (params.routingProtocol == RoutingType::DSDV) {
    NS_LOG_DEBUG("Using DSDV routing.");
    DsdvHelper dsdv;
    internet.SetRoutingHelper(dsdv);
  } else if (params.routingProtocol == RoutingType::AODV) {
    NS_LOG_DEBUG("Using AODV routing.");
    AodvHelper aodv;
    internet.SetRoutingHelper(aodv);
  }
  internet.Install(nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.0.0", "255.255.0.0");  // support up to 65534 devices in network.
  Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

  // install the application onto the nodes here
  SafApplicationHelper app(5000, params.totalNodes, params.totalDataItems);
  // any extra paramters would be set here

  app.SetAttribute("cache_hit_CB", CallbackValue(MakeCallback(&cache_hit_CB)));
  app.SetAttribute("lookup_sent_CB", CallbackValue(MakeCallback(&lookup_sent_CB)));
  app.SetAttribute("lookup_rcv_CB", CallbackValue(MakeCallback(&lookup_rcv_CB)));
  app.SetAttribute("lookup_rsp_sent_CB", CallbackValue(MakeCallback(&lookup_rsp_sent_CB)));
  app.SetAttribute("lookup_timeout_CB", CallbackValue(MakeCallback(&lookup_timeout_CB)));
  app.SetAttribute("realloc_timeout_CB", CallbackValue(MakeCallback(&realloc_timeout_CB)));
  app.SetAttribute("realloc_sent_CB", CallbackValue(MakeCallback(&realloc_sent_CB)));
  app.SetAttribute("realloc_rcv_CB", CallbackValue(MakeCallback(&realloc_rcv_CB)));
  app.SetAttribute("realloc_rsp_sent_CB", CallbackValue(MakeCallback(&realloc_rsp_sent_CB)));
  app.SetAttribute("lookup_ontime_CB", CallbackValue(MakeCallback(&lookup_ontime_CB)));
  app.SetAttribute("lookup_late_CB", CallbackValue(MakeCallback(&lookup_late_CB)));
  app.SetAttribute("realloc_ontime_CB", CallbackValue(MakeCallback(&realloc_ontime_CB)));
  app.SetAttribute("realloc_late_CB", CallbackValue(MakeCallback(&realloc_late_CB)));

  app.SetAttribute("NumNodes", UintegerValue(params.totalNodes));
  app.SetAttribute("TotalDataItems", UintegerValue(params.totalDataItems));
  app.SetAttribute("RequestTimeout", TimeValue(params.requestTimeout));
  app.SetAttribute("ReallocationPeriod", TimeValue(params.relocationPeriod));
  app.SetAttribute("DataSize", UintegerValue(params.dataSize));
  app.SetAttribute("accessFrequencyMode", UintegerValue(params.accessFrequencyType));
  app.SetAttribute("standardDeviation", DoubleValue(params.standardDeviation));
  app.SetAttribute("StorageSpace", UintegerValue(params.replicaSpace));

  ApplicationContainer apps = app.Install(nodes);

  // pick a start and end time that makes sense, maybe wait a little for the network to get setup
  // or something
  apps.Start(params.startupDelay);
  apps.Stop(params.runtime);

  // actually run the simulation
  // AnimationInterface anim( "animation-test.xml");
  // anim.SetMobilityPollInterval(Seconds(1));
  // AnimationInterface anim(params.netanimTraceFilePath);

  // AsciiTraceHelper ascii;
  // wifiPhy.EnableAsciiAll(ascii.CreateFileStream("saf.tr"));
  // wifiPhy.EnablePcapAll("saf", false);

  // actually run the simulation
  Simulator::Stop(params.runtime);
  Simulator::Run();
  Ptr<DataOutputInterface> output = CreateObject<OmnetDataOutput>();
  output->Output(data);
  Simulator::Destroy();

  return 0;
}
