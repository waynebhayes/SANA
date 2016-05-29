/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  shortest_path.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:45:14 $


#ifndef SHORTEST_PATH_H
#define SHORTEST_PATH_H


/*{\Manpage{shortest_path}{}{Shortest Path Algorithms}}*/

#include <LEDA/graph/graph.h>
#include <LEDA/core/stack.h>
#include <LEDA/core/queue.h>
#include <LEDA/core/list.h>

LEDA_BEGIN_NAMESPACE

/*{\Mtext
\newcommand{\Vp}{V^+}
\newcommand{\Vm}{V^-}
\newcommand{\Vf}{V^f}

\setlength{\setspacing}{0.1em}
\settowidth{\typewidth}{|void|}
\addtolength{\typewidth}{\colsep}
\computewidths

Let $G$ be a graph, $s$ a node in $G$, and |c| a cost 
function on the edges of $G$.  Edge costs may be positive or negative.
For a node $v$ let $\mu(v)$ be the length of 
a shortest path from $s$ to $v$ (more precisely, the infimum of the lengths
of all paths from $s$ to $v$). If $v$ is not reachable from $s$ then 
$\mu(v) = +\infty$ and if $v$ is reachable from $s$ through a cycle of negative
cost then $\mu(v) = -\infty$. Let $\Vp$, $\Vf$, and $\Vm$
 be the set of nodes $v$  with $\mu(v) = +\infty$, $-\infty < \mu(v) <+\infty$,
and $\mu(v) = -\infty$, respectively. 

The solution to a single source shortest path problem $(G,s,|c|)$ is a 
pair $(|dist|,|pred|)$ where |dist| is a |node_array<NT>| and |pred| is a 
|node_array<edge>| with the following properties. 
Let $P = \set{|pred[v]|}{v \in V \mbox{ and } |pred[v]| \not=
|nil|}$. A $P$-cycle is a cycle all of whose edges
belong to $P$ and a $P$-path is a path all of whose edges belong to $P$.

\begin{itemize}
\item $v \in \Vp$ iff $v \not= s$ and $|pred[v]| = |nil|$ and
$v \in \Vf \cup \Vm$ iff $v = s$ or $|pred[v]| \not= |nil|$.
\item $s \in \Vf$ if $|pred[s]| = |nil|$ and $s \in \Vm$ otherwise.
\item $v \in \Vf$ if $v$ is reachable from $s$ by a $P$-path and 
$s \in \Vf$. $P$ restricted to $\Vf$ forms a shortest path tree and 
$|dist[v]| = \mu(s,v)$ for $v \in \Vf$.
\item All $P$-cycles have negative cost and $v \in \Vm$
iff $v$ lies on a $P$-cycle or is reachable from a $P$-cycle
by a $P$-path.
\end{itemize}

Most functions in this section are template functions. The template parameter  
|NT| can be instantiated with any number type. In order to use the template
version of the function the .h-file
 
\verb+#include <LEDA/graph/templates/shortest_path.h>+ 

\noindent 
must be included. 
The functions are pre-instantiated with |int| and |double|. The function names
of the pre-instantiated versions are without the suffix \verb+_T+. 


Special care should be taken when using the functions with a number
type |NT| that can incur rounding error, e.g., the type |double|. 
The functions perform correctly if all arithmetic performed is 
without rounding error. This is the case if all numerical values in the input
are integers (albeit stored as a number of type |NT|) and 
if none of the intermediate
results exceeds the maximal integer representable by the 
number type ($2^{52}$ in the case of |doubles|).
All intermediate results are sums and differences
of input values, in particular, the algorithms do not use divisions and
multiplications. All intermediate values are bounded by $nC$ where $n$ is the 
number of nodes and $C$ is the maximal absolute value of any edge cost. 


}*/





template <class NT>
bool SHORTEST_PATH_T(const graph& G, node s, 
                     const edge_array<NT>& c, 
                     node_array<NT>& dist, 
                     node_array<edge>& pred );
/*{\Mfuncl
SHORTEST\_PATH solves the single source shortest path problem in the 
graph $G(V,E)$ with respect to the source node $s$ and the cost-function given
by the edge\_array |c|. 

The procedure returns false if there is a negative
cycle in $G$ that is reachable from $s$ and returns true otherwise.

It runs in linear time on acyclic graph, in time $O(m + n \log n)$ if all edge
costs are non-negative, and runs in time $O(\min(D,n)m)$ otherwise. Here $D$ is the
maximal number of edges on any shortest path. 
}*/

extern __exportF
list<edge> COMPUTE_SHORTEST_PATH(const graph& G, node s, node t, 
								 const node_array<edge>& pred);
/*{\Mfuncl
computes a shortest path from |s| to |t| assuming that |pred| 
stores a valid shortest path tree with root |s| (as it can be computed with the
previous function).
The returned list contains the edges on a shortest path from |s| to |t|.
The running time is linear in the length of the path.
}*/



template <class NT>
node_array<int> CHECK_SP_T(const graph& G, node s, 
                           const edge_array<NT>& c, 
                           const node_array<NT>& dist,
                           const node_array<edge>& pred);
/*{\Mfunc checks whether the pair
$(|dist|,|pred|)$ is a correct solution to the shortest path
problem $(G,s,|c|)$ and returns a |node_array<int>| |label|
with |label[v] < 0| if $v$ has distance $-\infty$ 
($-2$ for nodes lying on a negative 
cycle and $-1$ for a node reachable from a negative cycle), |label[v] = 0| 
if $v$ has finite distance, and 
|label[v] > 0| if $v$ has distance $+\infty$. 
The program aborts if the check fails. The algorithm takes linear time.
}*/







template <class NT>
void ACYCLIC_SHORTEST_PATH_T(const graph& G, node s, 
                             const edge_array<NT>& c, 
                             node_array<NT>& dist,
                             node_array<edge>& pred);
/*{\Mfunc solves the single source shortest path problem with respect
to source $s$. The algorithm takes linear time.\\
\precond $G$ must be acyclic. \\
  }*/ 




template <class NT>
void DIJKSTRA_T(const graph& G, node s, const edge_array<NT>& cost,
                    node_array<NT>& dist, node_array<edge>& pred);
/*{\Mfunc solves the shortest path problem in a graph with non-negative edges weights.\\
\precond The costs of all edges are non-negative. }*/
 

 
template <class NT>
void DIJKSTRA_T(const graph& G, node s, const edge_array<NT>& cost,
                    node_array<NT>& dist);
/*{\Mfunc as above, but |pred| is not computed.}*/
 


template<class NT>
NT DIJKSTRA_T(const graph& G, node s, node t, 
              const edge_array<NT>& c,
              node_array<edge>& pred);
/*{\Mfunc computes a shortest path from $s$ to $t$ and returns its length.
The cost of all edges must be non-negative. 
The return value is unspecified if there is no path from $s$ to $t$.
The array |pred| records a shortest path from $s$ to $t$ in
reverse order, i.e., |pred[t]| is the last edge on the path. If there is no
path from $s$ to $t$ or if $s = t$ then $|pred[t]| = |nil|$. The 
worst case running time is $O(m+n\log n)$, but frequently much better.}*/


template<class NT>
NT DIJKSTRA_T(const graph& G, node s, node t, 
              const edge_array<NT>& c,
              node_array<NT>& dist,
              node_array<edge>& pred);



template <class NT>
bool BELLMAN_FORD_B_T(const graph& G, node s, 
                        const edge_array<NT>& c, 
                        node_array<NT>& dist, 
                        node_array<edge>& pred); 
/*{\Mfuncl
BELLMAN\_FORD\_B solves the single source shortest path problem in the 
graph $G(V,E)$ with respect to the source node $s$ and the cost-function given
by the edge\_array |c|. 

BELLMAN\_FORD\_B returns false if there is a negative
cycle in $G$ that is reachable from $s$ and returns true otherwise.
The algorithm (\cite{Be58}) has running time $O(\min(D,n)m)$ where $D$ is the
maximal number of edges on any shortest path. The algorithm is only included
for pedagogical purposes. 
}*/



extern __exportF void BF_GEN(GRAPH<int,int>& G, int n, int m,
                                                bool non_negative = true);

/*{\Mfunc generates a graph with at most $n$ nodes and at most $m$ edges.
The edge costs are stored as edge data. The running time of 
BELLMAN\_FORD\_B on this graph is $\Omega(nm)$. The edge weights are 
non-negative if |non_negative| is true and are arbitrary otherwise.\\
\precond $m \ge 2n$ and $m \le n^2/2$.
}*/

template <class NT>
bool BELLMAN_FORD_T(const graph& G, node s, 
                       const edge_array<NT> & c,
                       node_array<NT> & dist,
                       node_array<edge> & pred);
/*{\Mfunc 
BELLMAN\_FORD\_T solves the single source shortest path problem in the 
graph $G(V,E)$ with respect to the source node $s$ and the cost-function given
by the edge\_array |c|. 

BELLMAN\_FORD\_T returns false if there is a negative
cycle in $G$ that is reachable from $s$ and returns true otherwise.
The algorithm (\cite{Be58}) has running time $O(\min(D,n)m)$ where $D$ is the
maximal number of edges in any shortest path. 

The algorithm is never significantly slower than BELLMAN\_FORD\_B and 
frequently much faster. 
}*/
LEDA_END_NAMESPACE


#include <LEDA/graph/node_matrix.h>


LEDA_BEGIN_NAMESPACE

template <class NT>
bool ALL_PAIRS_SHORTEST_PATHS_T(graph& G, 
                              const edge_array<NT>& c, 
                              node_matrix<NT>& DIST);
/*{\Mfunc
returns |true| if $G$ has no negative cycle and returns |false| 
otherwise. In the latter case 
all values returned in |DIST| are unspecified. In the
former case the following holds for all $v$ and $w$: if 
$\mu(v,w) < \infty$ then $|DIST(v,w)| = \mu(v,w)$ and if  $\mu(v,w) = \infty$
then the value of |DIST(v,w)| is arbitrary.
 The procedure 
runs in time 
$O(nm+n^2\log n)$.
}*/
LEDA_END_NAMESPACE

#include <LEDA/numbers/rational.h>

LEDA_BEGIN_NAMESPACE

extern __exportF rational MINIMUM_RATIO_CYCLE(graph& G, 
                                              const edge_array<int>& c,
                                              const edge_array<int>& p,
                                              list<edge>& C_star);
/*{\Mfunc Returns a minimum cost to profit ratio cycle |C_star| and the
ratio of the cycle. For a cycle $C$ let $c(C)$ be the sum of the $c$-values
of the edges on the cycle and let $p(C)$ be the sum of the $p$-values of 
the edges on the cycle. The cost to profit ratio of the cycle is the
quotient $c(C)/p(C)$. The cycle |C_start| realizes the minimum ratio for
any cycle $C$.
 The procedure 
runs in time 
$O(nm\log(n \cdot C \cdot P))$ where $C$ and $P$ are the maximum cost and
profit of any edge, respectively. The program returns zero if there
is no cycle in $G$.\\
\precond There are no cycles of cost zero or less with respect to either
$c$ or $p$.
}*/



template <class NT>
void  MOORE_T(const graph& g, node s, const edge_array<NT>& cost,
            node_array<NT>& dist, node t);

LEDA_END_NAMESPACE

#include <LEDA/graph/instant/shortest_path.h>


#endif

