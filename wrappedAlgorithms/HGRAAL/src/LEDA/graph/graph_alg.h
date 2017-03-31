/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  graph_alg.h
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
#define LEDA_ROOT_INCL_ID 500123
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/graph/graph.h>
#include <LEDA/graph/node_matrix.h>


/* extract as Manual Manpage */
/*{\Mtext
\bigskip
This chapter gives a summary of the graph algorithms contained in LEDA,
basic graph algorithms for reachability problems, shortest path algorithms,
matching algorithms, flow algorithms, \ldots\ .

All graph algorithms are generic, i.e., they accept 
instances of any user defined
parameterized graph type $GRAPH${\tt <}$vtype,etype${\tt >} as arguments.

All graph algorithms are available by including the 
header file {\tt <}LEDA/graph/graph\_alg.h{\tt >}. Alternatively, one may include
a more specific header file. 

An important subclass of graph algorithms are network algorithms. 
The input to most network algorithms is a graph whose edges or nodes
are labeled with numbers, e.g., shortest path algorithms get edge costs, 
network flow algorithms get edge capacities, and min cost flow algorithms
get edge capacities and edge costs. We use |NT| to denote the number
type used for the edge and node labels.

Most network algorithms come in three kinds: A templated version in which 
|NT| is a template parameter, and reinstantiated and precompiled 
versions for the number types
|int| (always) and |double| (except for a small number of functions). 
The function name of the templated version
ends in \_T. Thus MAX\_FLOW\_T is the name of the templated version of the max 
flow algorithm and MAX\_FLOW is the name of the instantiated version. 


In order to use the templated version a file {\tt <}LEDA/graph/templates/XXX.h{\tt >}
must be included, e.g., in order to use the templated version of the 
maxflow algorithm, one must include {\tt <}LEDA/graph/templates/max\_flow.h{\tt >}

Special care should be taken when using network algorithms with a number
type |NT| that can incur rounding error, e.g., the type |double|. 
The functions perform correctly if 
the arithmetic is exact.
This is the case if all numerical values in the input
are integers (albeit stored as a number of type |NT|), 
if none of the intermediate
results exceeds the maximal integer representable by the 
number type ($2^{52}$ in the case of |doubles|), and if no round-off errors
occur during the computation. We give more specific information on the 
arithmetic demand for each function below.
If the arithmetic incurs rounding error, the computation may fail in 
two ways: give a wrong answer or run forever. 

\bigskip
}*/



// basic graph algorithms

#include <LEDA/graph/basic_graph_alg.h>

// shortest path algorithms

#include <LEDA/graph/shortest_path.h>

// max flow algorithms

#include <LEDA/graph/max_flow.h>

// min cost flow

#include <LEDA/graph/min_cost_flow.h>

// minimum cuts

#include <LEDA/graph/min_cut.h>


// maximum cardinality matchings in bipartite graph
#include <LEDA/graph/mcb_matching.h>    

// maximum weight matchings in bipartite graph
#include <LEDA/graph/mwb_matching.h>

// maximum cardinality matching in general graph
#include <LEDA/graph/mc_matching.h>

// maximum weight matchings in general graph
#include <LEDA/graph/mw_matching.h>

// stable matching
#include <LEDA/graph/st_matching.h>

// minimum spanning trees
#include <LEDA/graph/min_span.h>

// Euler tours
#include <LEDA/graph/euler_tour.h>


// planar graph

#include <LEDA/graph/plane_graph_alg.h>  


// graph drawing
#include <LEDA/graph/graph_draw.h>

#if LEDA_ROOT_INCL_ID == 500123
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif


#endif
