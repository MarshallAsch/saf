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

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"

#include "ns3/application-container.h"
#include "ns3/attribute.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"

#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

#include "ns3/aodv-helper.h"
#include "ns3/wifi-module.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-helper.h"

#include "saf-helper.h"
#include "simulation-params.h"
#include "util.h"
#include "nsutil.h"

using namespace ns3;
using namespace saf;

bool verbose = true;

NS_LOG_COMPONENT_DEFINE("SAF runner");

void runWired() {
  Address serverAddress;

  //
  // Explicitly create the nodes required by the topology (shown above).
  //
  NS_LOG_INFO("Create nodes.");
  NodeContainer n;
  n.Create(4);

  InternetStackHelper internet;
  internet.Install(n);

  NS_LOG_INFO("Create channels.");
  //
  // Explicitly create the channels required by the topology (shown above).
  //
  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", DataRateValue(DataRate(5000000)));
  csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));
  csma.SetDeviceAttribute("Mtu", UintegerValue(1400));
  NetDeviceContainer d = csma.Install(n);

  //
  // We've got the "hardware" in place.  Now we need to add IP addresses.
  //
  NS_LOG_INFO("Assign IP Addresses.");

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign(d);
  serverAddress = Address(i.GetAddress(1));

  NS_LOG_INFO("Create Applications.");
  // install the application onto the nodes here
  SafApplicationHelper app(5000, n.GetN(), n.GetN());
  app.SetAttribute("ReallocationPeriod", UintegerValue(5));
  // any extra paramters would be set here

  ApplicationContainer apps = app.Install(n);

  // pick a start and end time that makes sense, maybe wait a little for the network to get setup or
  // something
  apps.Start(Seconds(1));
  apps.Stop(Seconds(20));

  // UdpEchoServerHelper2 server (5000);
  // apps = server.Install (n.Get(1));
  // apps.Start (Seconds (1.0));
  // apps.Stop (Seconds (20.0));

  AsciiTraceHelper ascii;
  csma.EnableAsciiAll(ascii.CreateFileStream("usaf-wire.tr"));
  csma.EnablePcapAll("saf-wire", false);

  //
  // Now, do the actual simulation.
  //
  NS_LOG_INFO("Run Simulation.");
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");
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

  // this will set a seed so that the same numbers are not generated each time.
  // the run number should be incremented each time this simulation is run to ensure streams do not
  // overlap
  RngSeedManager::SetSeed(params.seed);
  RngSeedManager::SetRun(params.runNumber);

  runWired();
  return 0;

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
  wifiChannel.AddPropagationLoss(
      "ns3::LogDistancePropagationLossModel",
      "Exponent",
      DoubleValue(3),
      "ReferenceDistance",
      DoubleValue(1.0),
      "ReferenceLoss",
      DoubleValue(41.7));
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

  ApplicationContainer apps = app.Install(nodes);

  // pick a start and end time that makes sense, maybe wait a little for the network to get setup
  // or something
  apps.Start(params.startupDelay);
  apps.Stop(params.runtime);

  // actually run the simulation
  // AnimationInterface anim( "animation-test.xml");
  // anim.SetMobilityPollInterval(Seconds(1));
  AnimationInterface anim(params.netanimTraceFilePath);

  AsciiTraceHelper ascii;
  wifiPhy.EnableAsciiAll(ascii.CreateFileStream("saf.tr"));
  wifiPhy.EnablePcapAll("saf", false);

  // actually run the simulation
  Simulator::Stop(params.runtime);
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
