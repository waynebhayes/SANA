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

#include <lemon/smart_graph.h>
#include <lemon/list_graph.h>
#include <lemon/lgf_reader.h>
#include <lemon/error.h>

#include "test_tools.h"

using namespace std;
using namespace lemon;

void digraph_copy_test() {
  const int nn = 10;

  // Build a digraph
  SmartDigraph from;
  SmartDigraph::NodeMap<int> fnm(from);
  SmartDigraph::ArcMap<int> fam(from);
  SmartDigraph::Node fn = INVALID;
  SmartDigraph::Arc fa = INVALID;

  std::vector<SmartDigraph::Node> fnv;
  for (int i = 0; i < nn; ++i) {
    SmartDigraph::Node node = from.addNode();
    fnv.push_back(node);
    fnm[node] = i * i;
    if (i == 0) fn = node;
  }

  for (int i = 0; i < nn; ++i) {
    for (int j = 0; j < nn; ++j) {
      SmartDigraph::Arc arc = from.addArc(fnv[i], fnv[j]);
      fam[arc] = i + j * j;
      if (i == 0 && j == 0) fa = arc;
    }
  }

  // Test digraph copy
  ListDigraph to;
  ListDigraph::NodeMap<int> tnm(to);
  ListDigraph::ArcMap<int> tam(to);
  ListDigraph::Node tn;
  ListDigraph::Arc ta;

  SmartDigraph::NodeMap<ListDigraph::Node> nr(from);
  SmartDigraph::ArcMap<ListDigraph::Arc> er(from);

  ListDigraph::NodeMap<SmartDigraph::Node> ncr(to);
  ListDigraph::ArcMap<SmartDigraph::Arc> ecr(to);

  digraphCopy(from, to).
    nodeMap(fnm, tnm).arcMap(fam, tam).
    nodeRef(nr).arcRef(er).
    nodeCrossRef(ncr).arcCrossRef(ecr).
    node(fn, tn).arc(fa, ta).run();

  check(countNodes(from) == countNodes(to), "Wrong copy.");
  check(countArcs(from) == countArcs(to), "Wrong copy.");

  for (SmartDigraph::NodeIt it(from); it != INVALID; ++it) {
    check(ncr[nr[it]] == it, "Wrong copy.");
    check(fnm[it] == tnm[nr[it]], "Wrong copy.");
  }

  for (SmartDigraph::ArcIt it(from); it != INVALID; ++it) {
    check(ecr[er[it]] == it, "Wrong copy.");
    check(fam[it] == tam[er[it]], "Wrong copy.");
    check(nr[from.source(it)] == to.source(er[it]), "Wrong copy.");
    check(nr[from.target(it)] == to.target(er[it]), "Wrong copy.");
  }

  for (ListDigraph::NodeIt it(to); it != INVALID; ++it) {
    check(nr[ncr[it]] == it, "Wrong copy.");
  }

  for (ListDigraph::ArcIt it(to); it != INVALID; ++it) {
    check(er[ecr[it]] == it, "Wrong copy.");
  }
  check(tn == nr[fn], "Wrong copy.");
  check(ta == er[fa], "Wrong copy.");

  // Test repeated copy
  digraphCopy(from, to).run();

  check(countNodes(from) == countNodes(to), "Wrong copy.");
  check(countArcs(from) == countArcs(to), "Wrong copy.");
}

void graph_copy_test() {
  const int nn = 10;

  // Build a graph
  SmartGraph from;
  SmartGraph::NodeMap<int> fnm(from);
  SmartGraph::ArcMap<int> fam(from);
  SmartGraph::EdgeMap<int> fem(from);
  SmartGraph::Node fn = INVALID;
  SmartGraph::Arc fa = INVALID;
  SmartGraph::Edge fe = INVALID;

  std::vector<SmartGraph::Node> fnv;
  for (int i = 0; i < nn; ++i) {
    SmartGraph::Node node = from.addNode();
    fnv.push_back(node);
    fnm[node] = i * i;
    if (i == 0) fn = node;
  }

  for (int i = 0; i < nn; ++i) {
    for (int j = 0; j < nn; ++j) {
      SmartGraph::Edge edge = from.addEdge(fnv[i], fnv[j]);
      fem[edge] = i * i + j * j;
      fam[from.direct(edge, true)] = i + j * j;
      fam[from.direct(edge, false)] = i * i + j;
      if (i == 0 && j == 0) fa = from.direct(edge, true);
      if (i == 0 && j == 0) fe = edge;
    }
  }

  // Test graph copy
  ListGraph to;
  ListGraph::NodeMap<int> tnm(to);
  ListGraph::ArcMap<int> tam(to);
  ListGraph::EdgeMap<int> tem(to);
  ListGraph::Node tn;
  ListGraph::Arc ta;
  ListGraph::Edge te;

  SmartGraph::NodeMap<ListGraph::Node> nr(from);
  SmartGraph::ArcMap<ListGraph::Arc> ar(from);
  SmartGraph::EdgeMap<ListGraph::Edge> er(from);

  ListGraph::NodeMap<SmartGraph::Node> ncr(to);
  ListGraph::ArcMap<SmartGraph::Arc> acr(to);
  ListGraph::EdgeMap<SmartGraph::Edge> ecr(to);

  graphCopy(from, to).
    nodeMap(fnm, tnm).arcMap(fam, tam).edgeMap(fem, tem).
    nodeRef(nr).arcRef(ar).edgeRef(er).
    nodeCrossRef(ncr).arcCrossRef(acr).edgeCrossRef(ecr).
    node(fn, tn).arc(fa, ta).edge(fe, te).run();

  check(countNodes(from) == countNodes(to), "Wrong copy.");
  check(countEdges(from) == countEdges(to), "Wrong copy.");
  check(countArcs(from) == countArcs(to), "Wrong copy.");

  for (SmartGraph::NodeIt it(from); it != INVALID; ++it) {
    check(ncr[nr[it]] == it, "Wrong copy.");
    check(fnm[it] == tnm[nr[it]], "Wrong copy.");
  }

  for (SmartGraph::ArcIt it(from); it != INVALID; ++it) {
    check(acr[ar[it]] == it, "Wrong copy.");
    check(fam[it] == tam[ar[it]], "Wrong copy.");
    check(nr[from.source(it)] == to.source(ar[it]), "Wrong copy.");
    check(nr[from.target(it)] == to.target(ar[it]), "Wrong copy.");
  }

  for (SmartGraph::EdgeIt it(from); it != INVALID; ++it) {
    check(ecr[er[it]] == it, "Wrong copy.");
    check(fem[it] == tem[er[it]], "Wrong copy.");
    check(nr[from.u(it)] == to.u(er[it]) || nr[from.u(it)] == to.v(er[it]),
          "Wrong copy.");
    check(nr[from.v(it)] == to.u(er[it]) || nr[from.v(it)] == to.v(er[it]),
          "Wrong copy.");
    check((from.u(it) != from.v(it)) == (to.u(er[it]) != to.v(er[it])),
          "Wrong copy.");
  }

  for (ListGraph::NodeIt it(to); it != INVALID; ++it) {
    check(nr[ncr[it]] == it, "Wrong copy.");
  }

  for (ListGraph::ArcIt it(to); it != INVALID; ++it) {
    check(ar[acr[it]] == it, "Wrong copy.");
  }
  for (ListGraph::EdgeIt it(to); it != INVALID; ++it) {
    check(er[ecr[it]] == it, "Wrong copy.");
  }
  check(tn == nr[fn], "Wrong copy.");
  check(ta == ar[fa], "Wrong copy.");
  check(te == er[fe], "Wrong copy.");

  // Test repeated copy
  graphCopy(from, to).run();

  check(countNodes(from) == countNodes(to), "Wrong copy.");
  check(countEdges(from) == countEdges(to), "Wrong copy.");
  check(countArcs(from) == countArcs(to), "Wrong copy.");
}


int main() {
  digraph_copy_test();
  graph_copy_test();

  return 0;
}
