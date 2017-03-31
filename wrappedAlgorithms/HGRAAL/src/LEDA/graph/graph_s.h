/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  graph_s.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:45:09 $

#ifndef LEDA_GRAPH_S_H
#define LEDA_GRAPH_S_H

#include <LEDA/s_graph.h>

LEDA_BEGIN_NAMESPACE

typedef s_graph<basic_graph,0,0> graph;
typedef graph::node node;
typedef graph::edge edge;
typedef graph::face face;




#define forall_nodes(v,G)\
LEDA_FORALL_PREAMBLE \
for(void* loop_var=(G).first_node();\
LOOP_ASSIGN(v,loop_var), loop_var=(G).next_node(v), v != (G).stop_node();)

#define forall_edges(e,G)\
LEDA_FORALL_PREAMBLE \
for(void* loop_var=(G).first_edge();\
LOOP_ASSIGN(e,loop_var), loop_var=(G).next_edge(e), e != (G).stop_edge();)


#define forall_rev_nodes(v,G)\
LEDA_FORALL_PREAMBLE \
for(void* loop_var=(G).last_node();\
LOOP_ASSIGN(v,loop_var), loop_var=(G).prev_node(v), v != (G).stop_node();)

#define forall_rev_edges(e,G)\
LEDA_FORALL_PREAMBLE \
for(void* loop_var=(G).last_edge();\
LOOP_ASSIGN(e,loop_var), loop_var=(G).prev_edge(e), e != (G).stop_edge();)


#define forall_out_edges(e,v)\
LEDA_FORALL_PREAMBLE \
for(void *loop_var=First_Adj_Edge(v,0), *loop_var1=Stop_Adj_Edge(v,0);\
LOOP_ASSIGN(e,loop_var), loop_var=Succ_Adj_Edge(e,0), (void*)e!=loop_var1;)

#define forall_adj_edges(e,v) forall_out_edges(e,v)

#define forall_in_edges(e,v)\
LEDA_FORALL_PREAMBLE \
for(void *loop_var=First_Adj_Edge(v,1), *loop_var1=Stop_Adj_Edge(v,1);\
LOOP_ASSIGN(e,loop_var), loop_var=Succ_In_Edge(e), (void*)e!=loop_var1;)


#define forall_inout_edges(e,v)\
LEDA_FORALL_PREAMBLE \
for(int loop_var=0; loop_var==0; loop_var++)\
for(void* loop_var1=First_Inout_Edge(v,loop_var),\
*loop_var2=Stop_Adj_Edge(v,loop_var);\
LOOP_ASSIGN(e,loop_var1), loop_var1=Succ_Inout_Edge(e,v,loop_var),\
(void*)e!=loop_var2; )




#include <LEDA/graph/node_array.h>
#include <LEDA/graph/edge_array.h>
#include <LEDA/graph/node_map.h>
#include <LEDA/graph/edge_map.h>


template <class vtype, class etype>
class GRAPH : public graph {

  node_array<vtype> ND;
  edge_array<etype> ED;
public:  
  GRAPH() { ND.init(*this); ED.init(*this); }

  node_array<vtype>& node_data() { return ND; }
  edge_array<etype>& edge_data() { return ED; }

  vtype& operator[](node v) { return ND[v]; }
  etype& operator[](edge e) { return ED[e]; }

  const vtype& operator[](node v) const { return ND[v]; }
  const etype& operator[](edge e) const { return ED[e]; }
};






LEDA_END_NAMESPACE

#endif
