/*
 * demo.cpp
 *
 *  Created on: 19-feb-2012
 *      Author: M. El-Kebir
 */

#include <lemon/smart_graph.h>
#include "natalie.h"
#include "input/identityparser.h"
#include "input/bpidentityparser.h"

using namespace lemon;
using namespace nina;
using namespace nina::gna;

typedef SmartGraph Graph;
typedef SmartBpGraph BpGraph;
typedef Natalie<Graph, BpGraph> NatalieType;
typedef IdentityParser<Graph> IdentityParserType;
typedef IdentityParserType::IdNodeMap IdNodeMap;
typedef IdentityParserType::WeightEdgeMap WeightEdgeMap;
typedef BpParser<Graph, BpGraph> BpParserType;
typedef BpParserType::BpWeightEdgeMap BpWeightEdgeMap;
typedef BpIdentityParser<Graph, BpGraph> BpIdentityParserType;
typedef BpIdentityParserType::MatchNodeToOrigNodeMap MatchNodeToOrigNodeMap;
typedef BpIdentityParserType::OrigNodeToMatchNodeMap OrigNodeToMatchNodeMap;

int main(int argc, char** argv)
{
  Graph g1;
  Graph g2;

  IdNodeMap idMap1(g1);
  IdNodeMap idMap2(g2);

  Graph::Node A = g1.addNode(), B = g1.addNode(), C = g1.addNode(), D = g1.addNode();
  idMap1[A] = "A";
  idMap1[B] = "B";
  idMap1[C] = "C";
  idMap1[D] = "D";

  Graph::Node a = g2.addNode(), b = g2.addNode(), c = g2.addNode(), d = g2.addNode(), e = g2.addNode();
  idMap2[a] = "a";
  idMap2[b] = "b";
  idMap2[c] = "c";
  idMap2[d] = "d";
  idMap2[e] = "e";

  WeightEdgeMap weightMap1(g1);
  WeightEdgeMap weightMap2(g2);

  Graph::Edge edge = g1.addEdge(A, B);
  weightMap1[edge] = 20;
  edge = g1.addEdge(A, C);
  weightMap1[edge] = 20;
  edge = g1.addEdge(A, D);
  weightMap1[edge] = 20;
  edge = g1.addEdge(B, C);
  weightMap1[edge] = 20;
  edge = g1.addEdge(B, D);
  weightMap1[edge] = 20;

  edge = g2.addEdge(a, b);
  weightMap2[edge] = 20;
  edge = g2.addEdge(a, d);
  weightMap2[edge] = 20;
  edge = g2.addEdge(a, e);
  weightMap2[edge] = 20;
  edge = g2.addEdge(b, c);
  weightMap2[edge] = 20;
  edge = g2.addEdge(b, d);
  weightMap2[edge] = 20;
  edge = g2.addEdge(b, e);
  weightMap2[edge] = 20;
  edge = g2.addEdge(c, d);
  weightMap2[edge] = 20;
  edge = g2.addEdge(d, e);
  weightMap2[edge] = 20;

  IdentityParserType parserG1(g1, NULL, &weightMap1, &idMap1);
  IdentityParserType parserG2(g2, NULL, &weightMap2, &idMap2);

  BpGraph gm;
  BpWeightEdgeMap weightGm(gm);
  MatchNodeToOrigNodeMap gmToG12(gm);
  OrigNodeToMatchNodeMap g1ToGm(g1);
  OrigNodeToMatchNodeMap g2ToGm(g2);
  
  BpGraph::RedNode bp_A = gm.addRedNode();
  gmToG12[bp_A] = A;
  g1ToGm[A] = bp_A;

  BpGraph::RedNode bp_B = gm.addRedNode();
  gmToG12[bp_B] = B;
  g1ToGm[B] = bp_B;

  BpGraph::RedNode bp_C = gm.addRedNode();
  gmToG12[bp_C] = C;
  g1ToGm[C] = bp_C;

  BpGraph::RedNode bp_D = gm.addRedNode();
  gmToG12[bp_D] = D;
  g1ToGm[D] = bp_D;

  BpGraph::BlueNode bp_a = gm.addBlueNode();
  gmToG12[bp_a] = a;
  g2ToGm[a] = bp_a;

  BpGraph::BlueNode bp_b = gm.addBlueNode();
  gmToG12[bp_b] = b;
  g2ToGm[b] = bp_b;

  BpGraph::BlueNode bp_c = gm.addBlueNode();
  gmToG12[bp_c] = c;
  g2ToGm[c] = bp_c;

  BpGraph::BlueNode bp_d = gm.addBlueNode();
  gmToG12[bp_d] = d;
  g2ToGm[d] = bp_d;

  BpGraph::BlueNode bp_e = gm.addBlueNode();
  gmToG12[bp_e] = e;
  g2ToGm[e] = bp_e;

  for (Graph::NodeIt n1(g1); n1 != lemon::INVALID; ++n1)
  {
    for (Graph::NodeIt n2(g2); n2 != lemon::INVALID; ++n2)
    {
      BpGraph::Edge bp_edge = gm.addEdge(gm.asRedNode(g1ToGm[n1]), gm.asBlueNode(g2ToGm[n2]));
      if (n1 == A && n2 == a)
        weightGm[bp_edge] = 2;
      else if (n1 == B && n2 == b)
        weightGm[bp_edge] = 2;
      else if (n1 == C && n2 == c)
        weightGm[bp_edge] = 2;
      else if (n1 == D && n2 == d)
        weightGm[bp_edge] = 2;
      else
        weightGm[bp_edge] = 0;
    }
  }

  NatalieType::Options options;
  options._beta = .5;
  options._normalize = false;

  //BpParserType parserGm(&parserG1, &parserG2);
  BpIdentityParserType parserGm(gm, gmToG12, &weightGm, &parserG1, &parserG2);

  NatalieType natalie;

  natalie.init(&parserG1, &parserG2, &parserGm);
  natalie.solve();
  natalie.addOutput(NatalieType::BP_OUT_SIF);
  natalie.generateOutput();

  return 0;
}
