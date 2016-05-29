/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  graph_alg_c.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:45:06 $

#ifndef LEDA_GRAPHALG_H
#define LEDA_GRAPHALG_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500122
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/graph/graph.h>
#include <LEDA/system/event.h>

LEDA_BEGIN_NAMESPACE

class __exportC alg_five_color {

  node_array<list<node> >* _merged_nodes;
  node_array<node>*        _node_copy;

  public :

  EVENT1<alg_five_color&>           start_event;
  EVENT1<alg_five_color&>           finish_event;
  EVENT2<alg_five_color&,node>      touch_node_event;
  EVENT3<alg_five_color&,node,node> merge_nodes_event;
  EVENT2<alg_five_color&,node>      remove_node_event;
  EVENT3<alg_five_color&,node,int>  color_node_event;

  const list<node>& get_merged_nodes(node v) { 
    return (*_merged_nodes)[(*_node_copy)[v]]; 
  }

  void operator () (const graph& G, node_array<int>& C);
};

extern __exportD alg_five_color FIVE_COLOR;


#if LEDA_ROOT_INCL_ID == 500122
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif


LEDA_END_NAMESPACE

#endif

