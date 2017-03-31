/* 
 * output.h
 *
 *  Created on: 18-feb-2011
 *      Author: M. El-Kebir
 */

#ifndef OUTPUT_H_
#define OUTPUT_H_

#include <lemon/core.h>
#include <ostream>
#include <fstream>
#include <string>
#include "input/matchinggraph.h"
#include "lagrange/lagrangegna.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class Output
{
public:
  /// The graph type of the input graphs
  typedef GR Graph;
  /// The graph type of the bipartite matching graph
  typedef BGR BpGraph;
  /// What to output
  typedef enum {
                 MINIMAL,    // nodes and matching edges
                 ORIG_EDGES, // nodes, matching and original edges
                 FULL,       // all nodes, matching edges and all original edges
               } OutputType;

protected:
  typedef MatchingGraph<Graph, BpGraph> MatchingGraphType;
  typedef typename LagrangeGna<Graph, BpGraph>::BpMatchingMap BpMatchingMapType;

  const MatchingGraphType&  _matchingGraph;

public:
  Output(const MatchingGraphType& matchingGraph)
    : _matchingGraph(matchingGraph)
  {
  }

  virtual ~Output() {}

  virtual void write(const BpMatchingMapType& matchingMap,
                     OutputType outputType,
                     std::ostream& outFile) const = 0;

  virtual void write(const BpMatchingMapType& matchingMap,
                     OutputType outputType, 
                     const std::string& filename) const;

  virtual std::string getExtension() const = 0;
};

template<typename GR, typename BGR>
inline void Output<GR, BGR>::write(const BpMatchingMapType& matchingMap,
                                   OutputType outputType,
                                   const std::string& filename) const
{
  std::ofstream out((filename + getExtension()).c_str());
  write(matchingMap, outputType, out);
}

} // namespace gna
} // namespace nina

#endif /* OUTPUT_H_ */
