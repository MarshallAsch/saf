/// \file simulation-params.cc
/// \author Keefer Rourke <krourke@uoguelph.ca>
///
/// Copyright (c) 2020 by Keefer Rourke <krourke@uoguelph.ca>
/// Permission to use, copy, modify, and/or distribute this software for any
/// purpose with or without fee is hereby granted, provided that the above
/// copyright notice and this permission notice appear in all copies.
///
/// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
/// REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
/// AND FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
/// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
/// LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
/// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
/// PERFORMANCE OF THIS SOFTWARE.

#include <inttypes.h>
#include <cmath>
#include <utility>

#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"
#include "ns3/random-walk-2d-mobility-model.h"

#include "logging.h"
#include "simulation-params.h"
#include "util.h"

namespace saf {
using namespace ns3;

// static
std::pair<SimulationParameters, bool> SimulationParameters::parse(int argc, char* argv[]) {
  /* Default simulation values. */
  // Simulation run time.
  double optRuntime = 50000.0_seconds;  // 40.0_minutes;
  double optStartupDelay = 1.0_seconds;
  // Simulation seed.
  uint32_t optSeed = 1;

  // Simulation seed.
  uint32_t optRunNum = 1;

  // Node parameters.
  uint32_t optTotalNodes = 40;  // constant

  // Simulation area parameters.
  double optAreaWidth = 50.0_meters;   // constant
  double optAreaLength = 50.0_meters;  // constant

  // node movement speed
  double optMinSpeed = 1.0_mps;  // constant
  double optMaxSpeed = 1.0_mps;  // constant

  // set pause time for
  double optMinPause = 1.0_seconds;   // not described
  double optMaxPause = 10.0_seconds;  // not described

  double optWifiRadius = 7.0_meters;  // variable R

  double optRequestTimeout = 10.0_seconds;   // not described
  uint32_t optDataSize = 256;                // not described
  double optRelocationPeriod = 256.0_seconds;  // variable T
  uint16_t optTotalDataItems = 40;           // constant
  uint16_t optReplicaSpace = 10;             // variable C

  uint16_t optDataFrequencyType = 1;  // option 1, 2 or 3
  double optStandardDeviation = 0.0;  // makes case 3 == case1, only for case 3

  // Link and network parameters.
  std::string optRoutingProtocol = "dsdv";

  // Animation parameters.
  std::string animationTraceFilePath = "saf.xml";

  /* Setup commandline option for each simulation parameter. */
  CommandLine cmd;
  cmd.AddValue("run-time", "Simulation run time in seconds", optRuntime);
  cmd.AddValue("start-delay", "Number of seconds before the application starts", optStartupDelay);
  cmd.AddValue("seed", "Simulation seed", optSeed);
  cmd.AddValue("run", "Simulation run", optRunNum);
  cmd.AddValue("total-nodes", "Total number of nodes in the simulation", optTotalNodes);
  cmd.AddValue("area-width", "Width of the simulation area in meters", optAreaWidth);
  cmd.AddValue("area-length", "Length of the simulation area in meters", optAreaLength);
  cmd.AddValue("min-speed", "The minimum node speed in meters/second", optMinSpeed);
  cmd.AddValue("max-speed", "The maximum node speed in meters/second", optMaxSpeed);
  cmd.AddValue("min-pause", "The minimum node pause time in seconds", optMinPause);
  cmd.AddValue("max-pause", "The maximum node pause time in seconds", optMaxPause);
  cmd.AddValue("wifi-radius", "The radius of connectivity for each node in meters", optWifiRadius);

  cmd.AddValue(
      "request-timeout",
      "The amount of time before a message request times out in seconds",
      optRequestTimeout);

  cmd.AddValue("data-size", "Number of bytes that make up a data item", optDataSize);

  cmd.AddValue(
      "relocation-period",
      "The amount of time between when the relocation process runs",
      optRelocationPeriod);

  cmd.AddValue(
      "data-items",
      "The total number of data items that will be stored",
      optTotalDataItems);
  cmd.AddValue(
      "replica-space",
      "The number of replicas that can be stored per node",
      optReplicaSpace);
  cmd.AddValue(
      "access-frequency-type",
      "Specify the access frequency algorithm to use [1,2, or 3]",
      optDataFrequencyType);

  cmd.AddValue(
      "standard-deviation",
      "The σ to use for tha calculation of the access frequencies for method 3",
      optStandardDeviation);

  cmd.AddValue("routing", "One of either 'DSDV' or 'AODV'", optRoutingProtocol);
  cmd.AddValue("animation-xml", "Output file path for NetAnim trace file", animationTraceFilePath);
  cmd.Parse(argc, argv);

  /* Parse the parameters. */

  bool ok = true;

  // maximum number of data items
  // ac 1 has a max number of data items of 100
  // ac 2 has a max number of data items of 40
  // ac 3 has a max number of data items of 100

  SimulationParameters result;
  if (optStartupDelay < 0) {
    NS_LOG_ERROR("startup delay(" << optStartupDelay << ") is cannot be negative");
    return std::pair<SimulationParameters, bool>(result, false);
  }
  if (optRuntime < 0) {
    NS_LOG_ERROR("simulation run time (" << optRuntime << ") is cannot be negative");
    return std::pair<SimulationParameters, bool>(result, false);
  }
  if (optDataFrequencyType > 3) {
    NS_LOG_ERROR(
        "Access frequency type (" << optDataFrequencyType
                                  << ") is not a valid, must be one of [1,2,3]");
    return std::pair<SimulationParameters, bool>(result, false);
  }
  if (optReplicaSpace > 150) {
    NS_LOG_ERROR(
        "Replica storage space (" << optReplicaSpace
                                  << ") is not valid, probably a signed overflow");
    return std::pair<SimulationParameters, bool>(result, false);
  }
  if (optRelocationPeriod < 0) {
    NS_LOG_ERROR("replica allocation period (" << optRelocationPeriod << ") is cannot be negative");
    return std::pair<SimulationParameters, bool>(result, false);
  }
  if (optRequestTimeout < 0) {
    NS_LOG_ERROR("request timeout (" << optRequestTimeout << ") is cannot be negative");
    return std::pair<SimulationParameters, bool>(result, false);
  }
  if (optMinPause < 0) {
    NS_LOG_ERROR("pause time minimum (" << optMinPause << ") is cannot be negative");
    return std::pair<SimulationParameters, bool>(result, false);
  }
  if (optMaxPause < optMinPause) {
    NS_LOG_ERROR("pause time max (" << optMaxPause << ") is cannot be less than the minimum");
    return std::pair<SimulationParameters, bool>(result, false);
  }
  if (optMinSpeed < 0 || optMinSpeed > 60) {
    NS_LOG_ERROR(
        "speed minimum (" << optMinSpeed << ") is cannot be negative, or greater than 60 m/s");
    return std::pair<SimulationParameters, bool>(result, false);
  }
  if (optMaxSpeed < optMinSpeed) {
    NS_LOG_ERROR("speed max (" << optMaxPause << ") is cannot be less than the minimum");
    return std::pair<SimulationParameters, bool>(result, false);
  }
  if (optAreaLength < 0) {
    NS_LOG_ERROR("simulation length (" << optAreaLength << ") is cannot be negative");
    return std::pair<SimulationParameters, bool>(result, false);
  }
  if (optAreaWidth < 0) {
    NS_LOG_ERROR("simulation width (" << optAreaWidth << ") is cannot be negative");
    return std::pair<SimulationParameters, bool>(result, false);
  }

  RoutingType routingType = getRoutingType(optRoutingProtocol);
  if (routingType == RoutingType::UNKNOWN) {
    NS_LOG_ERROR("Unrecognized routing type '" + optRoutingProtocol + "'.");
    return std::pair<SimulationParameters, bool>(result, false);
  }
  if (optTotalDataItems == 0) {
    NS_LOG_ERROR("Number of data items (" << optTotalDataItems << ") cannot be 0");
    return std::pair<SimulationParameters, bool>(result, false);
  }
  if (optTotalNodes == 0) {
    NS_LOG_ERROR("Number of nodes (" << optTotalNodes << ") cannot be 0");
    return std::pair<SimulationParameters, bool>(result, false);
  }
  if (optTotalDataItems % optTotalNodes != 0) {
    NS_LOG_ERROR(
        "Number of data items (" << optTotalDataItems
                                 << ") must be divisable by the number of nodes (" << optTotalNodes
                                 << ")");
    return std::pair<SimulationParameters, bool>(result, false);
  }

  result.seed = optSeed;
  result.runNumber = optRunNum;
  result.runtime = Seconds(optRuntime);

  // create position allocator
  Ptr<RandomRectanglePositionAllocator> positionAlloc =
      CreateObject<RandomRectanglePositionAllocator>();

  Ptr<RandomVariableStream> x = CreateObject<UniformRandomVariable>();
  Ptr<RandomVariableStream> y = CreateObject<UniformRandomVariable>();

  x->SetAttribute("Min", DoubleValue(0.0));
  x->SetAttribute("Max", DoubleValue(optAreaWidth));

  y->SetAttribute("Min", DoubleValue(0.0));
  y->SetAttribute("Max", DoubleValue(optAreaLength));

  positionAlloc->SetX(x);
  positionAlloc->SetY(y);
  positionAlloc->SetZ(0.0);  // all of the nodes are at ground level, do I need to change this?

  result.positionAllocator = positionAlloc;

  result.totalNodes = optTotalNodes;
  result.totalDataItems = optTotalDataItems;

  result.requestTimeout = Seconds(optRequestTimeout);
  result.relocationPeriod = Seconds(optRelocationPeriod);
  result.startupDelay = Seconds(optStartupDelay);

  // speed
  Ptr<RandomVariableStream> speed = CreateObject<UniformRandomVariable>();
  speed->SetAttribute("Min", DoubleValue(optMinSpeed));
  speed->SetAttribute("Max", DoubleValue(optMaxSpeed));
  result.speed = speed;

  // pause
  Ptr<RandomVariableStream> pause = CreateObject<UniformRandomVariable>();
  pause->SetAttribute("Min", DoubleValue(optMinPause));
  pause->SetAttribute("Max", DoubleValue(optMaxSpeed));
  result.pause = pause;

  result.replicaSpace = optReplicaSpace;
  result.dataSize = optDataSize;
  result.accessFrequencyType = optDataFrequencyType;
  result.standardDeviation = optStandardDeviation;

  result.routingProtocol = routingType;
  result.wifiRadius = optWifiRadius;
  result.netanimTraceFilePath = animationTraceFilePath;

  return std::pair<SimulationParameters, bool>(result, ok);
}
}  // namespace saf
