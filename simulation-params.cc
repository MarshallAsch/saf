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

#include "simulation-params.h"
#include "util.h"

namespace saf {
// static
std::pair<SimulationParameters, bool> SimulationParameters::parse(int argc, char* argv[]) {
  /* Default simulation values. */
  // Simulation run time.
  double optRuntime = 50000_seconds;  // 40.0_minutes;

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
  double optRelocationPeriod = 256_seconds;  // variable T
  uint16_t optTotalDataItems = 40;           // constant
  uint16_t optReplicaSpace = 10;             // variable C

  int optDataFrequencyType = 1            // option 1, 2 or 3
      double optStandardDeviation = 0.0;  // makes case 3 == case1, only for case 3

  // Link and network parameters.
  std::string optRoutingProtocol = "dsdv";

  // Animation parameters.
  std::string animationTraceFilePath = "saf.xml";

  /* Setup commandline option for each simulation parameter. */
  CommandLine cmd;
  cmd.AddValue("run-time", "Simulation run time in seconds", optRuntime);
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

  cmd.AddValue("wcol", "Weight of colocation in delivery probability calculations", optWcol);
  cmd.AddValue(
      "profile-update-delay",
      "Number of seconds between profile updates",
      optProfileUpdateDelay);

  cmd.AddValue("grid-rows", "Number of rows in the partition grid", optRows);
  cmd.AddValue("grid-cols", "Number of columns in the partition grid", optCols);
  cmd.AddValue("traveller-velocity", "Velocity of traveller nodes in m/s", optTravellerVelocity);
  cmd.AddValue(
      "traveller-walk-dist",
      "The distance in meters that traveller walks before changing "
      "directions",
      optTravellerWalkDistance);
  cmd.AddValue(
      "traveller-walk-time",
      "The time in seconds that should pass before a traveller changes "
      "directions",
      optTravellerWalkTime);
  cmd.AddValue(
      "traveller-walk-mode",
      "Should a traveller change direction after distance walked or time "
      "passed; options are 'distance' or 'time' ",
      optTravellerWalkMode);
  cmd.AddValue(
      "pbn-velocity-min",
      "Minimum velocity of partition-bound-nodes in m/s",
      optPbnVelocityMin);
  cmd.AddValue(
      "pbn-velocity-max",
      "Maximum velocity of partition-bound-nodes in m/s",
      optPbnVelocityMax);
  cmd.AddValue(
      "pbn-velocity-change-after",
      "Number of seconds after which each partition-bound node should change velocity",
      optPbnVelocityChangeAfter);
  cmd.AddValue("routing", "One of either 'DSDV' or 'AODV'", optRoutingProtocol);
  cmd.AddValue("animation-xml", "Output file path for NetAnim trace file", animationTraceFilePath);
  cmd.Parse(argc, argv);

  /* Parse the parameters. */

  bool ok = true;
  SimulationParameters result;
  if (optCarryingThreshold < 0 || optCarryingThreshold > 1) {
    NS_LOG_ERROR("Carrying threshold (" << optCarryingThreshold << ") is not a probability");
    return std::pair<SimulationParameters, bool>(result, false);
  }
  if (optForwardingThreshold < 0 || optForwardingThreshold > 1) {
    NS_LOG_ERROR("Forwarding threshold (" << optForwardingThreshold << ") is not a probability");
    return std::pair<SimulationParameters, bool>(result, false);
  }
  if (optWcol < 0 || optWcol > 1) {
    NS_LOG_ERROR("Colocation weight (" << optWcol << ") is not a probability");
    return std::pair<SimulationParameters, bool>(result, false);
  }
  if (optWcdc < 0 || optWcdc > 1) {
    NS_LOG_ERROR("Degree connectivity weight (" << optWcdc << ") is not a probability");
    return std::pair<SimulationParameters, bool>(result, false);
  }

  RandomWalk2dMobilityModel::Mode travellerWalkMode;
  std::tie(travellerWalkMode, ok) = getWalkMode(optTravellerWalkMode);
  if (!ok) {
    NS_LOG_ERROR("Unrecognized walk mode '" + optTravellerWalkMode + "'.");
  }
  if (!optTravellerWalkDistance) {
    optTravellerWalkDistance = std::min(optAreaWidth, optAreaLength);
  }

  RoutingType routingType = getRoutingType(optRoutingProtocol);
  if (routingType == RoutingType::UNKNOWN) {
    NS_LOG_ERROR("Unrecognized routing type '" + optRoutingProtocol + "'.");
    return std::pair<SimulationParameters, bool>(result, false);
  }

  if (optNodesPerPartition * optCols * optRows > optTotalNodes) {
    NS_LOG_ERROR(
        "Too few nodes (" << optTotalNodes << ") to populate all " << optCols * optRows
                          << " partitions with " << optNodesPerPartition << " nodes.");
    return std::pair<SimulationParameters, bool>(result, false);
  }

  if (optPercentageDataOwners < 0.0 || optPercentageDataOwners > 100.0) {
    NS_LOG_ERROR("percentage of data owners (" << optPercentageDataOwners << "%) is out of range");
    return std::pair<SimulationParameters, bool>(result, false);
  }

  Ptr<ConstantRandomVariable> travellerVelocityGenerator = CreateObject<ConstantRandomVariable>();
  travellerVelocityGenerator->SetAttribute("Constant", DoubleValue(optTravellerVelocity));

  Ptr<UniformRandomVariable> pbnVelocityGenerator = CreateObject<UniformRandomVariable>();
  pbnVelocityGenerator->SetAttribute("Min", DoubleValue(optPbnVelocityMin));
  pbnVelocityGenerator->SetAttribute("Max", DoubleValue(optPbnVelocityMax));

  result.seed = optSeed;
  result.runtime = Seconds(optRuntime);
  result.area = SimulationArea(
      std::pair<double, double>(0.0, 0.0),
      std::pair<double, double>(optAreaWidth, optAreaLength));
  result.rows = optRows;
  result.cols = optCols;

  result.totalNodes = optTotalNodes;
  result.dataOwners = std::round(optTotalNodes * (optPercentageDataOwners / 100.0));

  result.travellerNodes = optTotalNodes - (optNodesPerPartition * (optRows * optCols));
  result.travellerVelocity = travellerVelocityGenerator;
  result.travellerDirectionChangePeriod = Seconds(optTravellerWalkTime);
  result.travellerDirectionChangeDistance = optTravellerWalkDistance;
  result.travellerWalkMode = travellerWalkMode;

  result.nodesPerPartition = optNodesPerPartition;
  result.pbnVelocity = pbnVelocityGenerator;
  result.pbnVelocityChangePeriod = Seconds(optPbnVelocityChangeAfter);

  result.routingProtocol = routingType;
  result.wifiRadius = optWifiRadius;
  result.carryingThreshold = optCarryingThreshold;
  result.forwardingThreshold = optForwardingThreshold;
  result.neighborhoodSize = optNeighborhoodSize;
  result.electionNeighborhoodSize = optElectionNeighborhoodSize;
  result.wcdc = optWcdc;
  result.wcol = optWcol;
  result.profileUpdateDelay = Seconds(optProfileUpdateDelay);

  result.netanimTraceFilePath = animationTraceFilePath;

  return std::pair<SimulationParameters, bool>(result, ok);
}
}  // namespace saf
