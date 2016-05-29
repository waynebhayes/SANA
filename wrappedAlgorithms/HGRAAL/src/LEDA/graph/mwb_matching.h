/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  mwb_matching.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:45:10 $


#ifndef MWB_MATCHING
#define MWB_MATCHING

#include <LEDA/graph/graph.h>

LEDA_BEGIN_NAMESPACE


/*{\Manpage{mwb_matching}{}{Bipartite Weighted Matchings and Assignments}}*/

/*{\Mtext

\settowidth{\typewidth}{|list<edge>|}
\addtolength{\typewidth}{\colsep}
\settowidth{\callwidth}{MAX}
\computewidths


We give functions 
\begin{itemize}
\item to compute maximum and minimum weighted matchings in 
bipartite graph,
\item to check the optimality of matchings, and
\item to scale edge weights, so as to avoid round-off errors in computations 
with the number type |double|.
\end{itemize}
All functions for computing maximum or minimum weighted matchings 
provide a proof of optimality in the form of a potential function |pot|;
see the chapter on bipartite weighted matchings of the 
LEDA book for a discussion of potential functions.


The functions in this section are template functions. The template parameter  
|NT| can be instantiated with any number type. In order to use the template
version of the function the appropriate .h-file 
must be included.
 
\verb+#include <LEDA/graph/templates/mwb_matching.h>+ 

There are pre-instantiations for the
number types |int| and |double|.The pre-instantiated versions have the 
same function 
names except for the suffix \verb+_T+. 
In order to use them either

\verb+#include <LEDA/graph/mwb_matching.h>+

or

\verb+#include <LEDA/graph/graph_alg.h>+ 

has to be included (the latter file includes the former). The connection
between template functions and pre-instantiated functions is discussed in
detail in the section ``Templates for Network Algorithms'' of the LEDA book.
The function names of the pre-instantiated versions and the template versions
only differ by an additional suffix \verb+_T+ in the names of the latter ones. 

Special care should be taken when using the template 
functions with a number
type |NT| that can incur rounding error, e.g., the type |double|.
The section ``Algorithms on Weighted Graphs and Arithmetic Demand'' 
of the LEDA book contains
a general discussion of this issue.
The template functions
are only guaranteed to perform correctly if all arithmetic performed is 
without rounding error. This is the case if all numerical values in the input
are integers (albeit stored as a number of type |NT|) and 
if none of the intermediate
results exceeds the maximal integer representable by the 
number type ($2^{53} - 1$ in the case of |doubles|).
All intermediate results are sums and differences
of input values, in particular, the algorithms do not use divisions and
multiplications. 

The algorithms have the following arithmetic demands. Let
$C$ be the maximal absolute value of any edge cost. If all weights are integral
then all intermediate values are bounded by $3C$ in the case of 
maximum weight matchings and by $4 n C$ in the case of the 
other matching algorithms. Let $f = 3$ in the former case and let $f = 4n$
in the latter case. 

The pre-instantiations for number type |double|
compute the optimal matching for a modified weight function |c1|, where
for every edge $e$
\[ |c1|[e]= |sign|(c[e]) \lfloor \Labs{ |c[e]| } \cdot S \rfloor / S \]
and $S$ is the largest power of two such that $S < 2^{53}/(f\cdot C)$.

The weight of the optimal matching for the modified weight function and the 
weight of the optimal matching for the original weight function differ by at
most $n \cdot f \cdot C \cdot 2^{-52}$.

}*/



template <class NT>
list<edge> MAX_WEIGHT_BIPARTITE_MATCHING_T(graph& G,
                                const edge_array<NT>& c,
                                node_array<NT>& pot);
/*{\Mfunc computes a matching of maximal cost and a potential function |pot|
that is tight with respect to |M|. The running time of the algorithm is 
$O(n\cdot (m  + n\ log\ n))$. The argument |pot| is optional.\\
\precond $G$ must be bipartite. 
\bigskip
}*/

template <class NT>
list<edge> MAX_WEIGHT_BIPARTITE_MATCHING_T(graph& G,
                               const edge_array<NT>& c);



template <class NT>
list<edge> MAX_WEIGHT_BIPARTITE_MATCHING_T(graph& G, 
                               const list<node>& A, 
                               const list<node>& B,    
                         const edge_array<NT>& c,  
                               node_array<NT>& pot);
/*{\Mfunc As above. It is assumed that the partition |(A, B)| witnesses that |G| 
is bipartite and that all edges of $G$ are directed from $A$ to $B$. If $A$ and
$B$ have different sizes then is is advisable that $A$ is the smaller set; in
general, this leads to smaller running time. The argument |pot| is optional.
\bigskip
}*/

template <class NT>
list<edge> MAX_WEIGHT_BIPARTITE_MATCHING_T(graph& G, 
                const list<node>& A, const list<node>& B, const edge_array<NT>& c);


template <class NT>
bool CHECK_MWBM_T(const graph& G, const edge_array<NT>& c,
        const list<edge>& M, const node_array<NT>& pot);
/*{\Mfunc
checks that |pot| is a tight feasible
potential function with respect to 
$M$ and that $M$ is a matching. Tightness of |pot| implies that $M$ is a
maximum weighted matching.
\bigskip
}*/




template <class NT>
list<edge> MAX_WEIGHT_ASSIGNMENT_T(graph& G,
                               const edge_array<NT>& c,
                               node_array<NT>& pot);
/*{\Mfunc computes a perfect 
matching of maximal cost and a potential function |pot|
that is tight with respect to |M|. The running time of the algorithm is 
$O(n\cdot (m  + n\ log\ n))$. If $G$ contains no perfect matching 
the empty set of edges is returned. The argument |pot| is optional.\\
\precond $G$ must be bipartite.
\bigskip
}*/



template <class NT>
list<edge> MAX_WEIGHT_ASSIGNMENT_T(graph& G, 
                 const list<node>& A, const list<node>& B,    const edge_array<NT>& c, 
                                    node_array<NT>& pot);
/*{\Mfunc As above. It is assumed that the partition |(A, B)| witnesses that |G| 
is bipartite and that all edges of $G$ are directed from $A$ to $B$. The argument |pot| is optional.\\ 
\bigskip
}*/


template <class NT>
bool CHECK_MAX_WEIGHT_ASSIGNMENT_T(const graph& G, const edge_array<NT>& c,
        const list<edge>& M, const node_array<NT>& pot);
/*{\Mfunc
checks that |pot| is a tight feasible
potential function with respect to 
$M$ and that $M$ is a perfect matching. Tightness of |pot| implies that $M$ is a
maximum cost assignment.
\bigskip
}*/



template <class NT>
list<edge> MIN_WEIGHT_ASSIGNMENT_T(graph& G,
                               const edge_array<NT>& c,
                               node_array<NT>& pot);
/*{\Mfunc computes a perfect 
matching of minimal cost and a potential function |pot|
that is tight with respect to |M|. The running time of the algorithm is 
$O(n\cdot (m  + n\ log\ n))$. If $G$ contains no perfect matching 
the empty set of edges is returned. The argument |pot| is optional.\\
\precond $G$ must be bipartite.
\bigskip
}*/

template <class NT>
list<edge> MIN_WEIGHT_ASSIGNMENT_T(graph& G,                                    
                      const list<node>& A, const list<node>& B,    
                      const edge_array<NT>& c,
                      node_array<NT>& pot);
/*{\Mfunc As above. It is assumed that the partition |(A, B)| witnesses that |G| 
is bipartite and that all edges of $G$ are directed from $A$ to $B$. The argument |pot| is optional.
\bigskip
}*/

template <class NT>
bool CHECK_MIN_WEIGHT_ASSIGNMENT_T(const graph& G, const edge_array<NT>& c,
        const list<edge>& M, const node_array<NT>& pot);
/*{\Mfunc
checks that |pot| is a tight feasible
potential function with respect to 
$M$ and that $M$ is a perfect matching. Tightness of |pot| implies that $M$ is a
minimum cost assignment.
\bigskip
}*/


template <class NT>
list<edge> MAX_WEIGHT_ASSIGNMENT_T(graph& G,
                               const edge_array<NT>& c);

template <class NT>
list<edge> MAX_WEIGHT_ASSIGNMENT_T(graph& G,                                   
                      const list<node>& A, const list<node>& B, const edge_array<NT>& c);

template <class NT>
list<edge> MIN_WEIGHT_ASSIGNMENT_T(graph& G,
                               const edge_array<NT>& c);


template <class NT>
list<edge> MIN_WEIGHT_ASSIGNMENT_T(graph& G,                                  
                      const list<node>& A, const list<node>& B,  const edge_array<NT>& c);


template <class NT>
list<edge> MWMCB_MATCHING_T(graph& G, 
                                         const list<node>& A, 
                                         const list<node>& B,  
                                  const edge_array<NT>& c, 
                                        node_array<NT>& pot);
/*{\Mfunc Returns a maximum weight matching among the matching of 
maximum cardinality. 
The potential function |pot| is tight with respect to a modified 
cost function which increases the cost of every edge by $L = 1 + 2kC$ 
where $C$ is the maximum absolute value of any weight and 
$k = \min(\Labs{A},\Labs{B})$. 
It is assumed that the partition |(A, B)| witnesses that |G| 
is bipartite and that all edges of $G$ are directed from $A$ to $B$. If $A$ and
$B$ have different sizes, it is advisable that $A$ is the smaller set; in
general, this leads to smaller running time. The argument |pot| is optional.
\bigskip
}*/

template <class NT>
list<edge> MWMCB_MATCHING_T(graph& G, 
                          const list<node>& A, 
                          const list<node>& B,
                          const edge_array<NT>& c);


extern __exportF
bool MWBM_SCALE_WEIGHTS(const graph& G, 
                        edge_array<double>& c);
/*{\Mfunc replaces $c[e]$ by |c1[e]| for every edge $e$, where |c1[e]|
was defined above and $f = 3$. This scaling function is appropriate for the
maximum weight matching algorithm. The function returns |false| 
if the scaling changed some weight, and returns |true| otherwise.
}*/




extern __exportF
bool MWA_SCALE_WEIGHTS(const graph& G, 
                       edge_array<double>& c);
/*{\Mfunc replaces $c[e]$ by |c1[e]| for every edge $e$, where |c1[e]|
was defined above and $f = 4n$. This scaling function should be used for the algorithm that compute minimum of maximum weight assignments or maximum weighted
matchings of maximum cardinality.
The function returns |false| if the scaling changed some weight, 
and returns |true| otherwise. 
}*/


/* definition of pre-instantiations */

#include <LEDA/graph/instant/mwb_matching.h>


LEDA_END_NAMESPACE

#endif

