/*
 * outputanalyse.h
 *
 *  Created on: 5-mar-2012
 *      Author: M.E. van der Wees
 */

#ifndef OUTPUTANALYSE_H_
#define OUTPUTANALYSE_H_

#include <ostream>
#include <fstream>
#include "score/scoremodel.h"
#include "analysis/analyze.h"
#include "output/output.h"
#include "output/outputdot.h"
#include "output/outputlgf.h"
#include "output/outputsif.h"
#include "output/outputgml.h"
#include "output/outputjson.h"
#include "output/outputneato.h"
#include "output/outputcsv.h"
#include "output/outputeda.h"
#include "output/outputnoa.h"
#include "output/outputstatamc.h"
#include "output/outputcompstatamc.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class OutputAnalyse : public Output<GR, BGR>
{
public:
  /// The graph type of the input graphs
  typedef GR Graph;
  /// The graph type of the bipartite matching graph
  typedef BGR BpGraph;
  /// Base class type
  typedef Output<Graph, BpGraph> Parent;

  using Parent::_matchingGraph;


private:
  TEMPLATE_GRAPH_TYPEDEFS(Graph);

  typedef typename BpGraph::Node BpNode;
  typedef typename BpGraph::Edge BpEdge;
  typedef typename BpGraph::NodeIt BpNodeIt;
  typedef typename BpGraph::EdgeIt BpEdgeIt;
  typedef typename BpGraph::IncEdgeIt BpIncEdgeIt;
  typedef typename BpGraph::RedNode BpRedNode;
  typedef typename BpGraph::BlueNode BpBlueNode;
  typedef typename BpGraph::RedNodeIt BpRedNodeIt;
  typedef typename BpGraph::BlueNodeIt BpBlueNodeIt;
  typedef typename Parent::MatchingGraphType MatchingGraphType;
  typedef typename Parent::BpMatchingMapType BpMatchingMapType;
  typedef ScoreModel<Graph, BpGraph> ScoreModelType;

  /// Type of the outputs
  typedef typename Parent::OutputType OutputType;
  typedef OutputLgf<Graph, BpGraph> OutputLgfType;
  typedef OutputSif<Graph, BpGraph> OutputSifType;
  typedef OutputDot<Graph, BpGraph> OutputDotType;
  typedef OutputGml<Graph, BpGraph> OutputGmlType;
  typedef OutputJson<Graph, BpGraph> OutputJsonType;
  typedef OutputNeato<Graph, BpGraph> OutputNeatoType;
  typedef OutputCsv<Graph, BpGraph> OutputCsvType;
  typedef OutputEda<Graph, BpGraph> OutputEdaType;
  typedef OutputNoa<Graph, BpGraph> OutputNoaType;
  typedef OutputStatAmc<Graph, BpGraph> OutputStatAmcType;
  typedef OutputCompStatAmc<Graph, BpGraph> OutputCompStatAmcType;

  /// Type of the analysis
  typedef Analyse<Graph, BpGraph> AnalyseType;
  /// Type of the node list
  typedef std::set<BpNode> BpNodeSet;
  /// Type of the bipartite node set iterator
  typedef typename BpNodeSet::const_iterator BpNodeSetIt;
  /// Mapping from connected component numbers to nodes
  typedef std::vector<BpNodeSet> BpNodeSetVector;
  /// Type of a matching map: maps a node to its matching edge
  typedef typename BpGraph::template NodeMap<BpEdge> BpMatchingMap;

  /// Type of a vector of output methods
  typedef std::vector<Parent*> OutputVector;
  /// Iterator type of output vector
  typedef typename OutputVector::const_iterator OutputVectorIt;

protected:
  const ScoreModelType* _pScoreModel;
  OutputVector _outputs;
  double _elapsedTime;
  double _upperBound;

public:
  OutputAnalyse(const MatchingGraphType& matchingGraph,
                const ScoreModelType* pScoreModel,
                double elapsedTime,
                double upperBound)
    : Parent(matchingGraph)
    , _pScoreModel(pScoreModel)
    , _outputs()
    , _elapsedTime(elapsedTime)
    , _upperBound(upperBound)
  {
  }

  virtual ~OutputAnalyse()
  {
    for (OutputVectorIt it = _outputs.begin(); it != _outputs.end(); it++)
      delete *it;
  }

  void write(const BpMatchingMapType& matchingMap,
             OutputType outputType,
             const std::string& filename) const;

  void write(const BpMatchingMapType& matchingMap,
             OutputType outputType,
             std::ostream& outFile) const {}

  std::string getExtension() const { return ".csv"; }

  const OutputVector& getOutputs() const
  {
    return _outputs;
  }

  void setOutputs(const OutputVector& outputs)
  {
    _outputs = outputs;
  }

  void addOutput(Parent* pOutput)
  {
    _outputs.push_back(pOutput);
  }
};

template<typename GR, typename BGR>
inline void OutputAnalyse<GR, BGR>::write(const BpMatchingMapType& matchingMap,
                                   OutputType outputType,
                                   const std::string& filename) const
{
  // Create connected component graph
  AnalyseType analyse(_matchingGraph, _pScoreModel);
  analyse.makeGc(matchingMap);
  int numberOfComponents = analyse.getNumberOfComponents();
  const BpNodeSetVector& bpNodeSetVector = analyse.getBpNodeSetVector();

  std::ofstream globalOutFile((filename + getExtension()).c_str());
  globalOutFile << "\"Component\"" << ","
                << "\"# Aligned pairs\"" << ","
                << "\"Conserved\"" << ","
                << "\"Non-conserved in human\"" << ","
                << "\"Non-conserved in mouse\"" << ","
                << "\"Score\""
                   //<< ","
              //  << "\"Sequence score\"" << ","
              //  << "\"Topology score\""
                << std::endl;

  // Iterate over components with at least n/2 gene pairs
  for (int i = 0; i < numberOfComponents; i++)
  {
    if (bpNodeSetVector[i].size() >= 8) //parameter n? // at least 3 components/4 gene-pairs in component
    {
      // this is safe, log_10(MAXINT) < 128
      char buf[128];
      sprintf(buf, "-%d", i+1);
      std::string component(buf);

      // Create component specific matching map
      BpMatchingMap componentMap(_matchingGraph.getGm(), lemon::INVALID);
      for (BpNodeSetIt it = bpNodeSetVector[i].begin(); it != bpNodeSetVector[i].end(); it++)
      {
        componentMap[*it] = matchingMap[*it];
      }

      for (OutputVectorIt it = _outputs.begin(); it != _outputs.end(); it++)
      {
        std::ofstream outFile((filename + component + (*it)->getExtension()).c_str());
        (*it)->write(componentMap, outputType, outFile);
      }

      globalOutFile << '"' << i + 1 << '"' << ","
                    << '"' << analyse.getAlignedPairCount(i) << '"' << ","
                    << '"' << analyse.getConservedEdgeCount(i) << '"' << ","
                    << '"' << analyse.getNonConservedG1EdgeCount(i) << '"' << ","
                    << '"' << analyse.getNonConservedG2EdgeCount(i) << '"' << ","
                    << '"' << analyse.getScore(i) << '"'
                 //   << ","
                 //   << '"' << analyse.getSequenceScore(i) << '"' << ","
                 //   << '"' << analyse.getTopologyScore(i) << '"'
                    << std::endl;

      OutputCompStatAmcType stat(_matchingGraph, *_pScoreModel, analyse.getScore(i));
      std::ofstream outFile2((filename + component + ".csv").c_str());
      stat.write(componentMap, outputType, outFile2);
      outFile2.close();
    }
  }

  globalOutFile.close();
}

} // namespace gna
} // namespace nina

#endif /* OUTPUTANALYSE_H_ */
