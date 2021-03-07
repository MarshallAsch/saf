/// \file simulation-params.h
/// \author Keefer Rourke <mail@krourke.org>
/// \brief All simulation parameters are declared in this file.
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

#ifndef __simulation_params_h
#define __simulation_params_h

#include <inttypes.h>
#include <utility>

#include "ns3/nstime.h"
#include "ns3/position-allocator.h"
#include "ns3/random-variable-stream.h"
#include "ns3/random-walk-2d-mobility-model.h"

#include "nsutil.h"

namespace saf {

// TODO: Add parameters for neighborhood sizes, tau and sigma.

/// \brief Encapsulates parsing parameters from the CommandLine.
///     Each public member of this class corresponds to a strongly typed object
///     built from one or more of the commandline arguments.
///     Default parameter values are defined in the SimulationParameters::parse
///     method.
/// \see SimulationParameters::parse
class SimulationParameters {
 public:
  /// Simulation RNG seed.
  uint32_t seed;
  // Rhe run that this simulation is
  uint32_t runNumber;
  /// Simulation runtime.
  ns3::Time runtime;
  // number of seconds to wait until the application should begin, this time is taken out of the
  // total run time
  ns3::Time startupDelay;
  // Total nodes used in the simulation.
  uint32_t totalNodes;
  /// Total data items that get stored
  uint16_t totalDataItems;

  // the position allocator for the mobility model
  ns3::Ptr<ns3::RandomRectanglePositionAllocator> positionAllocator;

  ns3::Time requestTimeout;
  ns3::Time relocationPeriod;

  ns3::Ptr<ns3::RandomVariableStream> speed;
  ns3::Ptr<ns3::RandomVariableStream> pause;

  /// The path on disk to output the NetAnim trace XML file for visualizing the
  /// results of the simulation.
  std::string netanimTraceFilePath;

  uint16_t replicaSpace;
  uint32_t dataSize;
  uint16_t accessFrequencyType;
  double standardDeviation;

  /// Indicates the type of routing to use for the simulation.
  saf::RoutingType routingProtocol;
  /// The radi`connectivity for each node.
  double wifiRadius;

  SimulationParameters() {}

  /// \brief Parses command line options to set simulation parameters.
  ///
  /// \param argc The number of command line options passed to the program.
  /// \param argv The raw program arguments.
  /// \return std::pair<SimulationParameters, bool>
  ///   If the boolean value is false, there was an error calling the program,
  ///   and so the construction of the simulation parameters does not make sense.
  ///   If it is true, construction succeeded and the simulation may run.
  ///
  static std::pair<SimulationParameters, bool> parse(int argc, char* argv[]);
};

}  // namespace saf

#endif
