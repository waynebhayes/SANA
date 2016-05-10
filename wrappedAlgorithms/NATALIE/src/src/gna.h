/*
 * gna.h
 *
 *  Created on: 17-feb-2012
 *      Author: M. El-Kebir
 */

#ifndef GNA_H_
#define GNA_H_

#include <string>
#include <lemon/time_measure.h>
#include "input/matchinggraph.h"
#include "output/output.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class PairwiseGlobalNetworkAlignment
{
public:
  /// The graph type of the input graphs
  typedef GR Graph;
  /// The graph type of the bipartite matching graph
  typedef BGR BpGraph;

  typedef typename BpGraph::Node BpNode;
  typedef typename BpGraph::Edge BpEdge;
  typedef typename BpGraph::NodeIt BpNodeIt;
  typedef typename BpGraph::EdgeIt BpEdgeIt;
  typedef typename BpGraph::IncEdgeIt BpIncEdgeIt;
  typedef typename BpGraph::RedNode BpRedNode;
  typedef typename BpGraph::BlueNode BpBlueNode;
  typedef typename BpGraph::RedNodeIt BpRedNodeIt;
  typedef typename BpGraph::BlueNodeIt BpBlueNodeIt;

  /// Type of a map assigning a boolean to every matching edge
  typedef typename BpGraph::template EdgeMap<bool> BpBoolMap;
  /// Type of a matching map: maps a node to its matching edge
  typedef typename BpGraph::template NodeMap<BpEdge> BpMatchingMap;
  /// Type of the matching graph
  typedef MatchingGraph<Graph, BpGraph> MatchingGraphType;
  /// Type of the output
  typedef Output<Graph, BpGraph> OutputType;
  /// Type of a vector of output methods
  typedef std::vector<OutputType*> OutputVector;
  /// Iterator type of output vector
  typedef typename OutputVector::const_iterator OutputVectorIt;
  /// Type of the score model
  typedef ScoreModel<Graph, BpGraph> ScoreModelType;

protected:
  MatchingGraphType* _pMatchingGraph;
  ScoreModelType* _pScoreModel;
  OutputVector _outputs;

public:
  PairwiseGlobalNetworkAlignment(ScoreModelType* pScoreModel);

  virtual ~PairwiseGlobalNetworkAlignment();

  const OutputVector& getOutputs() const
  {
    return _outputs;
  }

  void setOutputs(const OutputVector& outputs)
  {
    _outputs = outputs;
  }

  void addOutput(OutputType* pOutput)
  {
    _outputs.push_back(pOutput);
  }

  const MatchingGraphType* getMatchingGraph() const
  {
    return _pMatchingGraph;
  }

  MatchingGraphType* getMatchingGraph()
  {
    return _pMatchingGraph;
  }

  void setMatchingGraph(MatchingGraphType* pMatchingGraph)
  {
    _pMatchingGraph = pMatchingGraph;
  }

  const ScoreModelType* getScoreModel() const
  {
    return _pScoreModel;
  }

  ScoreModelType* getScoreModel()
  {
    return _pScoreModel;
  }

  void setScoreModel(ScoreModelType* pScoreModel)
  {
    _pScoreModel = pScoreModel;
  }

  virtual double computePValue(int n) = 0;

  virtual int solve(int nSolutions = 1) = 0;

  virtual int exists(double score, bool& result) = 0;

  virtual void getSolution(BpBoolMap& m, int i = 0) const = 0;

  virtual void getSolution(BpMatchingMap& m, int i = 0) const = 0;

  virtual int getNumberOfSolutions() const = 0;

  virtual void generateOutput(const typename OutputType::OutputType outType = OutputType::MINIMAL,
    const std::string& filename = std::string());

};

template<typename GR, typename BGR>
inline PairwiseGlobalNetworkAlignment<GR, BGR>::
  PairwiseGlobalNetworkAlignment(ScoreModelType* pScoreModel)
  : _pMatchingGraph(new MatchingGraphType())
  , _pScoreModel(pScoreModel)
  , _outputs()
{
}

template<typename GR, typename BGR>
inline PairwiseGlobalNetworkAlignment<GR, BGR>::~PairwiseGlobalNetworkAlignment()
{
  delete _pScoreModel;

  for (typename OutputVector::iterator it = _outputs.begin();
       it != _outputs.end(); it++)
  {
    delete *it;
  }

  delete _pMatchingGraph;
}

template<typename GR, typename BGR>
inline void PairwiseGlobalNetworkAlignment<GR, BGR>::
  generateOutput(const typename OutputType::OutputType outType,
                 const std::string& filename)
{
  BpMatchingMap matchingMap(_pMatchingGraph->getGm(), lemon::INVALID);

  int n = getNumberOfSolutions();
  for (int i = 0; i < n; i++)
  {
    getSolution(matchingMap, i);

    for (OutputVectorIt it = _outputs.begin(); it != _outputs.end(); it++)
    {
      if (filename.empty() || filename == "-")
      {
        // output to std::out
        (*it)->write(matchingMap, outType, std::cout);
      }
      else
      {
        // this is safe, log_10(MAXINT) < 128
        char buf[128];
        sprintf(buf, "-%d", i+1);
        std::string number(buf);

        std::string newFilename = filename + number;
        (*it)->write(matchingMap, outType, newFilename);
      }
    }
  }
}

} // namespace gna
} // namespace nina

#endif // GNA_H_
