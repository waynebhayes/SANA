/* -*- mode: C++; indent-tabs-mode: nil; -*-
 *
 * This file is a part of LEMON, a generic C++ optimization library.
 *
 * Copyright (C) 2003-2011
 * Egervary Jeno Kombinatorikus Optimalizalasi Kutatocsoport
 * (Egervary Research Group on Combinatorial Optimization, EGRES).
 *
 * Permission to use, modify and distribute this software is granted
 * provided that this copyright notice appears in all copies. For
 * precise terms see the accompanying LICENSE file.
 *
 * This software is provided "AS IS" with no warranty of any kind,
 * express or implied, and with no claim as to its suitability for any
 * purpose.
 *
 */

#include <iostream>

#include "test_tools.h"
#include <lemon/smart_graph.h>
#include <lemon/preflow.h>
#include <lemon/concepts/digraph.h>
#include <lemon/concepts/maps.h>
#include <lemon/lgf_reader.h>
#include <lemon/elevator.h>

using namespace lemon;

char test_lgf[] =
  "@nodes\n"
  "label\n"
  "0\n"
  "1\n"
  "2\n"
  "3\n"
  "4\n"
  "5\n"
  "6\n"
  "7\n"
  "8\n"
  "9\n"
  "@arcs\n"
  "    label capacity\n"
  "0 1 0     20\n"
  "0 2 1     0\n"
  "1 1 2     3\n"
  "1 2 3     8\n"
  "1 3 4     8\n"
  "2 5 5     5\n"
  "3 2 6     5\n"
  "3 5 7     5\n"
  "3 6 8     5\n"
  "4 3 9     3\n"
  "5 7 10    3\n"
  "5 6 11    10\n"
  "5 8 12    10\n"
  "6 8 13    8\n"
  "8 9 14    20\n"
  "8 1 15    5\n"
  "9 5 16    5\n"
  "@attributes\n"
  "source 1\n"
  "target 8\n";

void checkPreflowCompile()
{
  typedef int VType;
  typedef concepts::Digraph Digraph;

  typedef Digraph::Node Node;
  typedef Digraph::Arc Arc;
  typedef concepts::ReadMap<Arc,VType> CapMap;
  typedef concepts::ReadWriteMap<Arc,VType> FlowMap;
  typedef concepts::WriteMap<Node,bool> CutMap;

  typedef Elevator<Digraph, Digraph::Node> Elev;
  typedef LinkedElevator<Digraph, Digraph::Node> LinkedElev;

  Digraph g;
  Node n;
  Arc e;
  CapMap cap;
  FlowMap flow;
  CutMap cut;
  VType v;
  bool b;

  typedef Preflow<Digraph, CapMap>
            ::SetFlowMap<FlowMap>
            ::SetElevator<Elev>
            ::SetStandardElevator<LinkedElev>
            ::Create PreflowType;
  PreflowType preflow_test(g, cap, n, n);
  const PreflowType& const_preflow_test = preflow_test;

  const PreflowType::Elevator& elev = const_preflow_test.elevator();
  preflow_test.elevator(const_cast<PreflowType::Elevator&>(elev));
  PreflowType::Tolerance tol = const_preflow_test.tolerance();
  preflow_test.tolerance(tol);

  preflow_test
    .capacityMap(cap)
    .flowMap(flow)
    .source(n)
    .target(n);

  preflow_test.init();
  preflow_test.init(cap);
  preflow_test.startFirstPhase();
  preflow_test.startSecondPhase();
  preflow_test.run();
  preflow_test.runMinCut();

  v = const_preflow_test.flowValue();
  v = const_preflow_test.flow(e);
  const FlowMap& fm = const_preflow_test.flowMap();
  b = const_preflow_test.minCut(n);
  const_preflow_test.minCutMap(cut);

  ignore_unused_variable_warning(fm);
}

int cutValue (const SmartDigraph& g,
              const SmartDigraph::NodeMap<bool>& cut,
              const SmartDigraph::ArcMap<int>& cap) {

  int c=0;
  for(SmartDigraph::ArcIt e(g); e!=INVALID; ++e) {
    if (cut[g.source(e)] && !cut[g.target(e)]) c+=cap[e];
  }
  return c;
}

bool checkFlow(const SmartDigraph& g,
               const SmartDigraph::ArcMap<int>& flow,
               const SmartDigraph::ArcMap<int>& cap,
               SmartDigraph::Node s, SmartDigraph::Node t) {

  for (SmartDigraph::ArcIt e(g); e != INVALID; ++e) {
    if (flow[e] < 0 || flow[e] > cap[e]) return false;
  }

  for (SmartDigraph::NodeIt n(g); n != INVALID; ++n) {
    if (n == s || n == t) continue;
    int sum = 0;
    for (SmartDigraph::OutArcIt e(g, n); e != INVALID; ++e) {
      sum += flow[e];
    }
    for (SmartDigraph::InArcIt e(g, n); e != INVALID; ++e) {
      sum -= flow[e];
    }
    if (sum != 0) return false;
  }
  return true;
}

void initFlowTest()
{
  DIGRAPH_TYPEDEFS(SmartDigraph);

  SmartDigraph g;
  SmartDigraph::ArcMap<int> cap(g),iflow(g);
  Node s=g.addNode(); Node t=g.addNode();
  Node n1=g.addNode(); Node n2=g.addNode();
  Arc a;
  a=g.addArc(s,n1); cap[a]=20; iflow[a]=20;
  a=g.addArc(n1,n2); cap[a]=10; iflow[a]=0;
  a=g.addArc(n2,t); cap[a]=20; iflow[a]=0;

  Preflow<SmartDigraph> pre(g,cap,s,t);
  pre.init(iflow);
  pre.startFirstPhase();
  check(pre.flowValue() == 10, "The incorrect max flow value.");
  check(pre.minCut(s), "Wrong min cut (Node s).");
  check(pre.minCut(n1), "Wrong min cut (Node n1).");
  check(!pre.minCut(n2), "Wrong min cut (Node n2).");
  check(!pre.minCut(t), "Wrong min cut (Node t).");
}


int main() {

  typedef SmartDigraph Digraph;

  typedef Digraph::Node Node;
  typedef Digraph::NodeIt NodeIt;
  typedef Digraph::ArcIt ArcIt;
  typedef Digraph::ArcMap<int> CapMap;
  typedef Digraph::ArcMap<int> FlowMap;
  typedef Digraph::NodeMap<bool> CutMap;

  typedef Preflow<Digraph, CapMap> PType;

  Digraph g;
  Node s, t;
  CapMap cap(g);
  std::istringstream input(test_lgf);
  DigraphReader<Digraph>(g,input).
    arcMap("capacity", cap).
    node("source",s).
    node("target",t).
    run();

  PType preflow_test(g, cap, s, t);
  preflow_test.run();

  check(checkFlow(g, preflow_test.flowMap(), cap, s, t),
        "The flow is not feasible.");

  CutMap min_cut(g);
  preflow_test.minCutMap(min_cut);
  int min_cut_value=cutValue(g,min_cut,cap);

  check(preflow_test.flowValue() == min_cut_value,
        "The max flow value is not equal to the three min cut values.");

  FlowMap flow(g);
  for(ArcIt e(g); e!=INVALID; ++e) flow[e] = preflow_test.flowMap()[e];

  int flow_value=preflow_test.flowValue();

  for(ArcIt e(g); e!=INVALID; ++e) cap[e]=2*cap[e];
  preflow_test.init(flow);
  preflow_test.startFirstPhase();

  CutMap min_cut1(g);
  preflow_test.minCutMap(min_cut1);
  min_cut_value=cutValue(g,min_cut1,cap);

  check(preflow_test.flowValue() == min_cut_value &&
        min_cut_value == 2*flow_value,
        "The max flow value or the min cut value is wrong.");

  preflow_test.startSecondPhase();

  check(checkFlow(g, preflow_test.flowMap(), cap, s, t),
        "The flow is not feasible.");

  CutMap min_cut2(g);
  preflow_test.minCutMap(min_cut2);
  min_cut_value=cutValue(g,min_cut2,cap);

  check(preflow_test.flowValue() == min_cut_value &&
        min_cut_value == 2*flow_value,
        "The max flow value or the three min cut values were not doubled");


  preflow_test.flowMap(flow);

  NodeIt tmp1(g,s);
  ++tmp1;
  if ( tmp1 != INVALID ) s=tmp1;

  NodeIt tmp2(g,t);
  ++tmp2;
  if ( tmp2 != INVALID ) t=tmp2;

  preflow_test.source(s);
  preflow_test.target(t);

  preflow_test.run();

  CutMap min_cut3(g);
  preflow_test.minCutMap(min_cut3);
  min_cut_value=cutValue(g,min_cut3,cap);


  check(preflow_test.flowValue() == min_cut_value,
        "The max flow value or the three min cut values are incorrect.");

  initFlowTest();

  return 0;
}
