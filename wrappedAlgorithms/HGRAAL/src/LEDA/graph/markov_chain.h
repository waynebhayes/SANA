/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  markov_chain.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:45:09 $


#include <LEDA/graph/graph.h>
#include <LEDA/core/random_variate.h>

LEDA_BEGIN_NAMESPACE

/*{\Manpage {markov_chain} {} {Markov Chains} {M}}*/ 

class markov_chain{

/*{\Mdefinition We consider a Markov Chain to be a graph $G$ in which each edge has an 
associated non-negative integer weight $w[e]$. For every node (with at least one
outgoing edge) the total weight 
of the outgoing edges must be positive A random walk in a Markov chain starts at 
some node $s$ and then performs steps according to the following rule:

Initially, $s$ is the current node. 
Suppose node $v$ is the current node and that 
$e_0$, \ldots, $e_{d-1}$ are the edges out of $v$. If $v$ has no outgoing edge no further
step can be taken. Otherwise, the walk follows edge $e_i$
with probability proportional to $w[e_i]$ for all $i$, $0 \le i < d$. The target
node of the chosen edge becomes the new current node. }*/

const graph* G_ptr;
int    N;
node_array<int> visits;
node   vcur;
node_array<array<node> > neighbors;
node_array<random_variate*> variate;

public:

/*{\Mcreation}*/

markov_chain(const graph& G, const edge_array<int>& w, node s = nil)
/*{\Mcreate creates a Markov chain for the graph $G$ with edge weights $w$.
            The node $s$ is taken as the start vertex (|G.first_node()| 
            if $s$ is nil). }*/
  : visits(G,0), neighbors(G), variate(G)  // KM: changed visits(G) into visits(G,0)
{ 
  N = 0; 
  vcur = (s) ? s : G.first_node() ;
  G_ptr = &G;

/*
  visits = node_array<int>(G,0);
  neighbors = node_array<array<node> >(G);
  variate   = node_array<random_variate*>(G);
*/

  node v; 
  forall_nodes(v,G)
  { if (G.outdeg(v) == 0) continue;
    neighbors[v] = array<node>(G.outdeg(v));
    array<int> weights(G.outdeg(v));
    int i = 0;
    edge e;
    forall_adj_edges(e,v)
    { neighbors[v][i] = G.target(e);
      weights[i] = w[e];
      i++;
    }
    variate[v] = new random_variate(weights);
  }
 }

~markov_chain()
{ node v;
  forall_nodes(v,*G_ptr) delete variate[v];
 }

/*{\Moperations}*/

void step(int T = 1)
  /*{\Mop performs T steps of the Markov chain.}*/
{ 
  if (T <= 0 ) return;
  for (int i = 0; i < T; i++)
  { if ( G_ptr->outdeg(vcur) == 0) return;
    vcur = neighbors[vcur][variate[vcur] -> generate()];
    visits[vcur]++;
    N++;
  }
 }


node current_node(){ return vcur; }
/*{\Mop returns current vertex.}*/

int current_outdeg(){ return G_ptr->outdeg(vcur); }
/*{\Mop returns the outdegree of the current vertex.}*/

int number_of_steps(){ return N; }
/*{\Mop returns number of steps performed.}*/

int  number_of_visits(node v){ return visits[v]; }
/*{\Mop returns number of visits to node v.}*/

double rel_freq_of_visit(node v){ return ((double)visits[v])/N; }
/*{\Mop returns number of visits divided by the total number of steps.}*/


};


/*{\Mtext \newpage}*/

/*{\Manpage {dynamic_markov_chain} {} {Dynamic Markov Chains} {M}}*/ 

class dynamic_markov_chain{

/*{\Mdefinition A Markov Chain is a graph $G$ in which each edge has an 
associated non-negative integer weight $w[e]$. For every node (with at least one
outgoing edge) the total weight 
of the outgoing edges must be positive A random walk in a Markov chain starts at 
some node $s$ and then performs steps according to the following rule:

Initially, $s$ is the current node. 
Suppose node $v$ is the current node and that 
$e_0$, \ldots, $e_{d-1}$ are the edges out of $v$. If $v$ has no outgoing edge no further
step can be taken. Otherwise, the walk follows edge $e_i$
with probability proportional to $w[e_i]$ for all $i$, $0 \le i < d$. 
The target
node of the chosen edge becomes the new current node. 
}*/

const graph* G_ptr;
int    N;
node_array<int> visits;
node   vcur;
node_array<array<node> > neighbors;
node_array<dynamic_random_variate*> variate;
edge_array<int> index;

public:

/*{\Mcreation}*/

dynamic_markov_chain(const graph& G, const edge_array<int>& w, node s = nil)
/*{\Mcreate creates a Markov chain for the graph $G$ with edge weights $w$.
The node $s$ is taken as the start vertex (|G.first_node()| if $s$ is nil).}*/
  :  visits(G,0), neighbors(G), variate(G), index(G)   // KM: inserted this line
{ 
  N = 0; 
  vcur = (s) ? s : G.first_node() ;

  G_ptr = &G;

/*
  visits = node_array<int>(G,0);
  neighbors = node_array<array<node> >(G);
  variate   = node_array<dynamic_random_variate*>(G);
  index     = edge_array<int>(G);
*/

  node v; edge e;
  forall_nodes(v,G)
  { if (G.outdeg(v) == 0) continue;
    neighbors[v] = array<node>(G.outdeg(v));
    array<int> weights(G.outdeg(v));
    int i = 0;
    forall_adj_edges(e,v)
    { index[e] = i;
      neighbors[v][i] = G.target(e);
      weights[i] = w[e];
      i++;
    }
    variate[v] = new dynamic_random_variate(weights);
  }
 }

~dynamic_markov_chain()
{ node v;
  forall_nodes(v,*G_ptr) delete variate[v];
 }


/*{\Moperations}*/

void step(int T = 1)
  /*{\Mop performs T steps of the Markov chain.}*/
{ 
  if (T <= 0 ) return;
  for (int i = 0; i < T; i++)
  { if ( G_ptr->outdeg(vcur) == 0) return;
    vcur = neighbors[vcur][variate[vcur] -> generate()];
    visits[vcur]++;
    N++;
  }
 }


node current_node(){ return vcur; }
/*{\Mop returns current vertex.}*/

int current_outdeg(){ return G_ptr->outdeg(vcur); }
/*{\Mop returns the outdegree of the current vertex.}*/

int number_of_steps(){ return N; }
/*{\Mop returns number of steps performed.}*/

int  number_of_visits(node v){ return visits[v]; }
/*{\Mop returns number of visits to node v.}*/

double rel_freq_of_visit(node v){ return ((double)visits[v])/N; }
/*{\Mop returns number of visits divided by the total number of steps.}*/

int set_weight(edge e,int g)
{ return variate[G_ptr->source(e)]->set_weight(index[e],g); }
/*{\Mop changes the weight of edge $e$ to $g$ and 
returns the old weight of $e$}*/


};

LEDA_END_NAMESPACE


