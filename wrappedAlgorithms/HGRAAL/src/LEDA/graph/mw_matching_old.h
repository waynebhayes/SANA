/*******************************************************************************
+
+  LEDA 5.0.1
+
+
+  mw_matching_old.h
+
+
+  Copyright (c) 1995-2001 by  Algorithmic Solutions Software GmbH
+  Postfach 151101, 66041 Saarbruecken, Germany
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:45:10 $

#ifndef MW_MATCHING_OLD
#define MW_MATCHING_OLD

#include <LEDA/core/list.h>
#include <LEDA/core/tuple.h>
#include <LEDA/core/array.h>
#include <LEDA/graph/graph.h>

LEDA_BEGIN_NAMESPACE

typedef int NT;
typedef two_tuple<NT,int> QUAD;

bool __exportF CHECK_MAX_WEIGHT_MATCHING_OLD(const graph& G,
   const edge_array<NT>& weight,
   const list<edge>& M,
   const node_array<NT>& u,
   const array<QUAD>& BT,
   const node_array<int>& b);

list<edge> __exportF MAX_WEIGHT_MATCHING_OLD
 (const graph& G,
   const edge_array<NT>& weight,
   node_array<NT>& u,
   array<QUAD>& BT,
   node_array<int>& b
);

list<edge> __exportF MAX_WEIGHT_MATCHING_OLD(const graph& G, const edge_array<NT>& E, bool checked = false);

LEDA_END_NAMESPACE

#endif
