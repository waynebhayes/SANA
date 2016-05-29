/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  float_geo_alg.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:53 $


#ifndef LEDA_FLOAT_GEO_ALG_H
#define LEDA_FLOAT_GEO_ALG_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500096
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/system/basic.h>
#include <LEDA/geo/geo_global_enums.h>
#include <LEDA/core/list.h>
#include <LEDA/graph/graph.h>
#include <LEDA/geo/float_kernel.h>

LEDA_BEGIN_NAMESPACE

/*{\Manpage {geo_alg} {} {Geometry Algorithms} }*/

/*{\Mtext
\bigskip
All functions listed in this section work for geometric objects based
on both floating-point and exact (rational) arithmetic. In particular,
|point| can be replace by |rat_point|, |segment| by |rat_segment|,
and |circle| by |rat_circle|. 

The floating point versions are faster but unreliable. They may produce
incorrect results, abort, or run forever. Only the rational versions
will produce correct results for all inputs. 

The include-file for the rational version is rat\_geo\_alg.h, the
include-file for the floating point version is float\_geo\_alg.h, and 
geo\_alg.h includes both versions. Including both versions increases compile
time. An alternative name for geo\_alg.h is plane\_alg.h.
}*/


/*{\Mtext
\bigskip
$\bullet$ {\bf Convex Hulls}
\setopdims{2.65cm}{0.5cm}
}*/


extern __exportF list<point> CONVEX_HULL(const list<point>& L);
/*{\Mfuncl
CONVEX\_HULL takes as argument a list of points and returns the polygon
representing the convex hull of $L$. The cyclic order of the vertices in the
result list corresponds to counter-clockwise order of the vertices on the hull.
The algorithm calls our current favorite of the algorithms below.
}*/

extern __exportF polygon CONVEX_HULL_POLY(const list<point>& L);
/*{\Mfuncl
as above, but returns the convex hull of |L| as a polygon. 
}*/ 

extern __exportF list<point> UPPER_CONVEX_HULL(const list<point>& L);
/*{\Mfuncl
returns the upper convex hull of |L|.
}*/ 

extern __exportF list<point> LOWER_CONVEX_HULL(const list<point>& L);
/*{\Mfuncl
returns the lower convex hull of |L|.
}*/ 


extern __exportF list<point> CONVEX_HULL_S(const list<point>& L);
/*{\Mfuncl as above, but the algorithm is based on the sweep paradigm.
Running time is $O(n \log n)$ in the worst and in the best case. }*/

extern __exportF list<point> CONVEX_HULL_IC1(const list<point>& L);
extern __exportF list<point> CONVEX_HULL_IC2(const list<point>& L);

extern __exportF list<point> CONVEX_HULL_IC(const list<point>& L);
/*{\Mfuncl as above, but the algorithm is based on incremental construction.
The running time is $O(n^2)$ worst case and is $O(n \log n)$ expected case.
The expectation is computed as the average over all permutations of $L$.
The running time is linear in the best case.}*/

extern __exportF list<point> CONVEX_HULL_RIC(const list<point>& L);
/*{\Mfuncl as above. The algorithm permutes $L$ randomly and then calls the
preceding function. }*/

extern __exportF double WIDTH(const list<point>& L, line& l1, line& l2);
  /*{\Mfunc returns the square of the minimum width 
of a stripe covering all points in $L$
and the two boundaries of the stripe.\\
\precond $L$ is non-empty}*/

/*{\Mtext
\bigskip
$\bullet$ {\bf Halfplane intersections}
\setopdims{2.65cm}{0.5cm}
}*/
	     

extern __exportF void HALFPLANE_INTERSECTION(const list<line>& L, 
                                             list<line>& Lout);
/*{\Mfuncl
For every line $\ell\in L$ let $h_\ell$ be the closed halfplane lying on the 
positive side of $\ell$, i.e., 
$h_\ell = \{\ p\in\real^2\ \Lvert\ orientation(\ell,p) \ge 0\ \}$, 
and let $H = \cap_{\ell\in L} h_\ell$.
Then HALFPLANE\_INTERSECTION computes the list of lines |Lout| defining
the boundary of $H$ in counter-clockwise ordering.
}*/

extern __exportF void HALFPLANE_INTERSECTION(const list<line>& L, list<segment>& S);
/*{\Xfuncl
HALFPLANE\_INTERSECTION takes as the first argument a list of lines and computes the
intersection of the positive halfplanes $h_i$ defined by the lines $l_i$ in |L|
(the halfplanes left from a directed line).\\
The function returns the result in |S|. We distinguish several cases:
\begin{itemize}
\item The intersection of halfplanes is empty or 
the full plane. Then |S| will be empty.
\item The intersection of halfplanes is a polygon. Then |S| will be the cyclic list in 
counter-clockwise order of the  segments of that polygon. 
\item The intersection of halfplanes is a halfplane. Then |S| will contain one segment, and
the result halfplane will be the halfplane left to the segment.
\item The intersection of halfplanes is an open polygon. |S| will be the cyclic list in 
counter-clockwise order of the  segments of that polygon, and the "open" sides will be
missing.
\end{itemize}
}*/




//#include <LEDA/geo/geo_global_enums.h>

/*{\Mtext
\bigskip
$\bullet$ {\bf Triangulations}
\setopdims{1cm}{1.9cm}
}*/


extern __exportF int DELAUNAY_FLIPPING(GRAPH<point,segment>&);


extern __exportF edge TRIANGULATE_POINTS(const list<point>& L, GRAPH<point,int>& T);
/*{\Mfuncl computes a triangulation (planar map) $T$ of the points in $L$ and 
           returns an edge of the outer face (convex hull). }*/

extern __exportF int DELAUNAY_FLIPPING(GRAPH<point,int>& T,
                        delaunay_voronoi_kind k = NEAREST);
extern __exportF int DELAUNAY_FLIPPING(GRAPH<point,int>& T,list<edge> L,
                        delaunay_voronoi_kind k = NEAREST);
extern __exportF int DELAUNAY_FLIP(const list<point>&,GRAPH<point,int>&);
extern __exportF int F_DELAUNAY_FLIP(const list<point>&,GRAPH<point,int>&);

extern __exportF void DELAUNAY_STOLFI(const list<point>&,GRAPH<point,int>&,bool=false);
extern __exportF void DELAUNAY_DWYER (const list<point>&,GRAPH<point,int>&,bool=false);


extern __exportF void DELAUNAY_TRIANG(const list<point>& L, GRAPH<point,int>& DT);
/*{\Mfuncl computes the delaunay triangulation $DT$ of the points in $L$. }*/

extern __exportF void DELAUNAY_DIAGRAM(const list<point>& L, GRAPH<point,int>& DD);
/*{\Mfuncl computes the delaunay diagram $DD$ of the points in $L$. }*/



extern __exportF void F_DELAUNAY_TRIANG(const list<point>& L, GRAPH<point,int>& FDT);
/*{\Mfuncl computes the furthest point delaunay triangulation $FDT$ of the 
           points in $L$. }*/

extern __exportF void F_DELAUNAY_DIAGRAM(const list<point>& L, GRAPH<point,int>& FDD);
/*{\Mfuncl computes the furthest point delaunay diagram $FDD$ of the 
           points in $L$. }*/


/*{\Mtext
\bigskip
$\bullet$ {\bf Constraint Triangulations}
\setopdims{1cm}{1.9cm}
}*/

extern __exportF edge TRIANGULATE_SEGMENTS(const list<segment>& L, 
                                  GRAPH<point,segment>& G);
/*{\Mfuncl computes a constrained triangulation (planar map) $T$ of the 
           segments in $L$ (trivial segments representing points are allowed). 
	   The function returns an edge of the outer face (convex hull). }*/

extern __exportF edge DELAUNAY_TRIANG(const list<segment>& L, 
                                                     GRAPH<point,segment>& G);
/*{\Mfuncl computes a constrained Delaunay triangulation  $T$ of the 
           segments in $L$.  The function returns an edge of the outer 
           face (convex hull). }*/

extern __exportF edge TRIANGULATE_PLANE_MAP(GRAPH<point,segment>& G);
/*{\Mfuncl computes a constrained triangulation $T$ of the  plane map 
           (counter-clockwise straight-line embedded Graph) |G|. 
           The function returns an edge of the outer face (convex hull).
           \precond |G| is simple. }*/

extern __exportF edge DELAUNAY_TRIANG(GRAPH<point,segment>& G);
/*{\Mfuncl computes a constrained Delaunay triangulation $T$ of the 
           plane map |G|. The function returns an edge of the outer 
           face (convex hull). \precond |G| is simple. }*/



extern __exportF edge TRIANGULATE_POLYGON(const polygon& P,
                                                    GRAPH<point,segment>& G,
                                                    list<edge>& inner_edges,
                                                    list<edge>& outer_edges,
                                                    list<edge>& boundary_edges);
/*{\Mfuncl triangulates the interior and exterior of the simple polygon |P|
           and stores all edges of the inner (outer) triangulation in 
           |inner_edges| (|outer_edges|) and the edges of the polygon
           boundary in |boundary_edges|. The function returns an edge
           of the convex hull of |P| if |P| is simple and |nil| otherwise. }*/



extern __exportF edge TRIANGULATE_POLYGON(const gen_polygon& GP, 
                                                GRAPH<point,segment>& G, 
                                                list<edge>& inner_edges,
                                                list<edge>& outer_edges,
                                                list<edge>& boundary_edges,
                                                list<edge>& hole_edges);
/*{\Mfuncl triangulates the interior and exterior of the generalized polygon 
           |GP| and stores all edges of the inner (outer) triangulation in
           |inner_edges| (|outer_edges|). The function returns |nil| if 
           |GP| is trivial, and an edge of the convex hull otherwise. 
           |boundary_edges| contains the edges of every counter-clockwise 
           oriented boundary cycle of |GP|,
           and |hole_edges| contains the edges on every clockwise oriented 
           boundary cycle of |GP|. Note that the reversals of boundary 
           and hole edges will be returned in |inner_edges|.
	   \precond |GP| is simple. }*/			 
	
	   



extern __exportF edge CONVEX_COMPONENTS(const polygon& P, 
                                                   GRAPH<point,segment>& G,
                                                   list<edge>& inner_edges,
                                                   list<edge>& boundary);
/*{\Mfuncl if |P| is a bounded and non-trivial simple polygon
           its interior is decomposed into convex parts. All inner edges 
           of the constructed decomposition are returned in |inner_edges|.
	   |boundary_edges| contains the edges of the polygon boundary
           Note that the reversals of boundary edges will be 
           stored in |inner_edges|. The function returns an edge of the convex
           hull if |P| is simple and non-trivial and |nil| otherwise. }*/



extern __exportF edge CONVEX_COMPONENTS(const gen_polygon& GP, 
                                                     GRAPH<point,segment>& G,
                                                     list<edge>& inner_edges, 
                                 		     list<edge>& boundary_edges,
                                 		     list<edge>& hole_edges);		      
/*{\Mfuncl if |GP| is a bounded and non-trivial generalized polygon,
           its interior is decomposed into convex parts. All inner edges 
           of the constructed decomposition are returned in |inner_edges|.
	   |boundary_edges| contains the edges of every counter-clockwise 
           oriented boundary cycle of |GP|, and |hole_edges| contains the 
           edges of every clockwise oriented boundary cycle of |GP|. 
           Note that the reversals of boundary and hole edges will be 
           stored in |inner_edges|. The function returns an edge of the convex
           hull if |GP| is a bounded and non-trivial and |nil| otherwise.
           \precond |GP| must be simple. }*/



extern __exportF list<polygon> TRIANGLE_COMPONENTS(const gen_polygon& GP);
/*{\Mfunc triangulates the interior of generalized polygon |GP| and returns the
          result of the triangulation as a list of polygons. }*/

extern __exportF list<polygon> CONVEX_COMPONENTS(const gen_polygon& GP);
/*{\Mfunc if |GP|is a bounded and non-trivial generalized polygon, 
          its interior is decomposed into convex parts. The function 
          returns a list of polygons that form the convex decomposition 
          of |GP|s interior. }*/






/*{\Mtext
\bigskip
$\bullet$ {\bf Minkowski Sums}
\setopdims{1cm}{1.9cm}
Please note that the Minkowski sums only work reliable for the rational kernel.
}*/

extern __exportF gen_polygon MINKOWSKI_SUM(const polygon& P, const polygon& R);
/*{\Mfuncl computes the Minkowski sum of $P$ and $R$. }*/

extern __exportF gen_polygon MINKOWSKI_DIFF(const polygon& P, const polygon& R);
/*{\Mfuncl computes the Minkowski difference of $P$ and $R$, i.e. the
           Minkowski sum of $P$ and |R.reflect(point(0,0))|. }*/

extern __exportF gen_polygon MINKOWSKI_SUM(const gen_polygon& P, const polygon& R);
/*{\Mfuncl computes the Minkowski sum of $P$ and $R$. }*/

extern __exportF gen_polygon MINKOWSKI_DIFF(const gen_polygon& P, const polygon& R);
/*{\Mfuncl computes the Minkowski difference of $P$ and $R$, i.e. the
           Minkowski sum of $P$ and |R.reflect(point(0,0))|. }*/


/*{\Mtext
The following variants of the |MINKOWSKI| functions take two additional 
call-back function arguments |conv_partition| and |conv_unite| which are 
used by the algorithm to partition the input polygons into convex parts 
and for computing the union of a list of convex polygons, respectively
(instead of using the default methods).}*/


extern __exportF gen_polygon MINKOWSKI_SUM(const polygon& P, 
                                           const polygon& R,
                        void (*conv_partition)(const gen_polygon& p, 
                                               const polygon& r, 
                                               list<polygon>& lp, 
                                               list<polygon>& lr),
			gen_polygon (*conv_unite)(const list<gen_polygon>&));
/*{\Mfuncl 
}*/

extern __exportF gen_polygon MINKOWSKI_DIFF(const polygon& P, 
                                            const polygon& R,
                        void (*conv_partition)(const gen_polygon& p, 
                                               const polygon& r, 
                                               list<polygon>& lp, 
                                               list<polygon>& lr),
			gen_polygon (*conv_unite)(const list<gen_polygon>&));
/*{\Mfuncl 
}*/

extern __exportF gen_polygon MINKOWSKI_SUM(const gen_polygon& P, 
                                           const polygon& R,
                        void (*conv_partition)(const gen_polygon& p, 
                                               const polygon& r, 
                                               list<polygon>& lp, 
                                               list<polygon>& lr),
			gen_polygon (*conv_unite)(const list<gen_polygon>&));
/*{\Mfuncl 
}*/

extern __exportF gen_polygon MINKOWSKI_DIFF(const gen_polygon& P, 
                                            const polygon& R,
                        void (*conv_partition)(const gen_polygon& p, 
                                               const polygon& r, 
                                               list<polygon>& lp, 
                                               list<polygon>& lr),
			gen_polygon (*conv_unite)(const list<gen_polygon>&));
/*{\Mfuncl 
}*/


/*{\Mtext
\bigskip
$\bullet$ {\bf Euclidean Spanning Trees}
\setopdims{1cm}{1.9cm}
}*/

extern __exportF void MIN_SPANNING_TREE_OLD(const list<point>& L, GRAPH<point,int>& T);

extern __exportF void MIN_SPANNING_TREE(const list<point>& L, GRAPH<point,int>& T);
/*{\Mfuncl computes the Euclidian minimum spanning tree $T$ of the 
           points in $L$. }*/



/*{\Mtext
\bigskip
$\bullet$ {\bf Triangulation Checker}
\setopdims{1cm}{1.9cm}
}*/


extern __exportF bool Is_Convex_Subdivision(const GRAPH<point,int>& G);
/*{\Mfunc returns true if $G$ is a convex planar subdivision.}*/

extern __exportF bool Is_Triangulation(const GRAPH<point,int>& G);
/*{\Mfunc returns true if $G$ is convex planar subdivision in which every
bounded face is a triangle or if all nodes of $G$ lie on a common line.}*/



extern __exportF bool Is_Delaunay_Triangulation(const GRAPH<point,int>& G, 
                               delaunay_voronoi_kind kind);
/*{\Mfunc checks whether $G$ is a nearest (|kind = NEAREST|)
 or furthest (|kind = FURTHEST|)  site Delaunay triangulation of
its vertex set. $G$ is a Delaunay triangulation iff it is a 
triangulation and 
all triangles have the Delaunay property. A triangle has the 
Delaunay property if
no vertex of an adjacent triangle is contained in the 
interior (|kind = NEAREST|)
or exterior (|kind = FURTHEST|) of the triangle. }*/

extern __exportF bool Is_Delaunay_Diagram(const GRAPH<point,int>& G, 
                         delaunay_voronoi_kind kind);
/*{\Mfunc checks whether $G$ is a nearest (|kind = NEAREST|)
 or furthest (|kind = FURTHEST|)  site Delaunay diagram of
its vertex set. $G$ is a Delaunay diagram if it is a convex subdivision, 
if the vertices
of any bounded face are 
co-circular, and if every triangulation of $G$ is a Delaunay
triangulation.}*/




/*{\Mtext
\bigskip
$\bullet$ {\bf Voronoi Diagrams}
}*/

extern __exportF void DELAUNAY_TO_VORONOI(const GRAPH<point,int>& DT, 
                                          GRAPH<circle,point>& VD);

extern __exportF void VORONOI(const list<point>& L, GRAPH<circle,point>& VD);
/*{\Mfuncl 
VORONOI takes as input a list of points (sites) |L|. It computes a directed 
graph $VD$ representing the planar subdivision defined by the Voronoi diagram 
of |L|. For each node $v$ of |VD| |G[v]| is the corresponding Voronoi 
vertex ($point$) and for each edge $e$  |G[e]| is the site ($point$) 
whose Voronoi region is bounded by $e$. 
The algorithm has running time $O(n^2)$ in the worst case and $O(n\log n)$ 
with high probability, where $n$ is the number of sites.
}*/

//extern __exportF void VORONOI(const list<point>& L, subdivision<point>& VD);
/*\Xfuncl computes the Voronoi diagram of $L$ as a subdivision (see
data type |subdivision|) whose faces are labeled with the corresponding
voronoi sites. This representation of Voronoi diagrams supports efficient
nearest neighbor queries by performing a point location in the subdivision:
|f = VD.locate_point(p)| returns the  face $f$ of |VD| that contains $p$.
Then, the information associated with $f$ (|VD[f]|) is the site closest 
to $p$. }*/

  


extern __exportF void VORONOI(const list<point>& L, double R, GRAPH<point,point>& VD);


extern __exportF void F_DELAUNAY_TO_VORONOI(const GRAPH<point,int>& DT, 
                                     GRAPH<circle,point>& VD);


extern __exportF void F_VORONOI(const list<point>& L, GRAPH<circle,point>& FVD);
/*{\Mfuncl computes the farthest point Voronoi Diagram |FVD| of the points
           in |L|. }*/


extern __exportF circle  LARGEST_EMPTY_CIRCLE(const list<point>& L);
/*{\Mfuncl computes a largest circle whose center lies inside the convex
           hull of |L| that contains no point of |L| in its interior. 
           Returns the trivial circle if |L| is empty. }*/ 

extern __exportF circle  SMALLEST_ENCLOSING_CIRCLE(const list<point>& L);
/*{\Mfuncl computes a smallest circle containing all points of |L| in 
           its interior. }*/ 

extern __exportF void ALL_EMPTY_CIRCLES(const list<point>& L, list<circle>& CL);
/*{\Mfuncl computes the list |CL| of all empty circles passing through three
           or more points of |L|. }*/

extern __exportF void ALL_ENCLOSING_CIRCLES(const list<point>& L, list<circle>& CL);
/*{\Mfuncl computes the list |CL| of all enclosing circles passing through three
           or more points of |L|. }*/

/*{\Mtext An annulus is either the region between two concentric 
circles or the region between two parallel lines. }*/

extern __exportF bool MIN_AREA_ANNULUS(const list<point>& L, 
                      point& center, 
                      point& ipoint, 
                      point& opoint,
                      line&  l1);
/*{\Mfuncl computes the minimum area annulus containing the points of |L|. 
The function returns false if all points in $L$ are collinear and 
returns true otherwise. In the former case a line passing through the points
in $L$ is returned in |l1|, and in the latter case the annulus is returned
by its |center| and a point 
           on the inner and the outer circle, respectively.}*/



extern __exportF bool MIN_WIDTH_ANNULUS(const list<point>& L, 
                       point& center, 
                       point& ipoint,
                       point& opoint,
                       line& l1,
                       line& l2);
/*{\Mfuncl computes the minimum width annulus containing the points of |L|. 
The function returns false if the minimum width annulus is a stripe and returns
true otherwise. In the former case the boundaries of the stripes are returned 
in |l1| and |l2| and in the latter case the 
annulus is returned by its |center| and a point 
on the inner and the outer circle, respectively.}*/


inline bool MIN_AREA_ANNULUS(const list<point>& L, 
                             point& center, point& ipoint, point& opoint)
{ line l1;
  return MIN_AREA_ANNULUS(L,center,ipoint,opoint,l1);
 }


inline bool MIN_WIDTH_ANNULUS(const list<point>& L, 
                              point& center, point& ipoint, point& opoint)
{ line l1,l2;
  return MIN_WIDTH_ANNULUS(L,center,ipoint,opoint,l1,l2);
 }


extern __exportF void  CRUST(const list<point>& L0, GRAPH<point,int>& G);
/*{\Mfunc takes a list |L0| of points and traces to guess the curve(s) 
from which |L0| are sampled. The algorithm is due to Amenta, Bern, and 
Eppstein. The algorithm is guaranteed to succeed
if |L0| is a sufficiently dense sample from a smooth closed curve.}*/



bool Is_Voronoi_Diagram(const GRAPH<circle,point>& G, 
                        delaunay_voronoi_kind kind);
/*{\Mfunc checks whether $G$ represents a nearest (|kind = NEAREST|)
 or furthest (|kind = FURTHEST|) site Voronoi diagram. 

Voronoi diagrams of point sites are represented as 
planar maps as follows: There is a vertex for each vertex of the 
Voronoi diagram
and, in addition, a vertex ``at infinity'' for each ray of the Voronoi diagram.
Vertices at infinity have degree one. The edges of the graph correspond to the
edges of the Voronoi diagram. The chapter on Voronoi 
diagrams of the 
LEDA-book \cite{LEDAbook} contains more details.
Each edge is labeled with the site (class |POINT|)
owning the region to its left and each vertex is labeled with a
triple of points (= the three defining points of a |CIRCLE|). For 
a ``finite'' vertex
the three points are any three sites associated with regions incident 
to the vertex
(and hence the center of the circle is the position of the vertex in the plane)
and for a vertex at infinity the three points are collinear and the 
first point 
and the third point of the triple are the sites whose regions are 
incident to the
vertex at infinity. Let $a$ and $c$ be the first and third point of the 
triple respectively; $a$ and
$c$ encode the geometric position of the vertex at infinity as follows:
the vertex lies
on the perpendicular bisector of $a$ and $c$ and to the left of the 
segment $ac$.
\vspace{1ex}

}*/



/*{\Mtext
\bigskip
$\bullet$ {\bf Line Segment Intersection}
}*/



extern __exportF void MULMULEY_SEGMENTS(const list<segment>& S, 
                                        GRAPH<point,segment>& G, 
                                        bool embed=false);

inline void SEGMENT_INTERSECTION(const list<segment>& S, 
                                 GRAPH<point,segment>& G, 
                                 bool embed=false)
{ MULMULEY_SEGMENTS(S,G,embed); }
/*{\Mfuncl takes a list of segments $S$ as input and computes 
the planar graph $G$ induced by the set of straight line segments
in $S$. The nodes of $G$ are all endpoints and all proper intersection
points of segments in $S$. The edges of $G$ are the maximal relatively open
subsegments of segments in $S$ that contain no node of $G$. The edges are
directed as the corresponding segments. If the flag |embed| is true,
the corresponding planar map is computed. Note that for each edge $e$
$G[e]$ is the input segment that contains $e$ (see the LEDA book for details). 
}*/

extern __exportF void SWEEP_SEGMENTS(const list<segment>& S, 
                                     GRAPH<point,segment>& G, 
                                     bool embed=false,
                                     bool use_optimization = true);
/*{\Mfuncl
as above.\\
The algorithm (\cite{BO79}) runs in time $O((n+s)\log n) + m)$, where 
$n$ is the 
number of segments, $s$ is the number of vertices of the graph $G$, and
$m$ is the number of edges of $G$. If $S$ contains no overlapping segments
then $m = O(n + s)$. If |embed| is true the running time increases by
$O(m \log m)$. If |use_optimization| is true an optimization described in the 
LEDA book is used.   }*/

extern __exportF void SWEEP_SEGMENTS(const list<segment>&, list<point>&);



/*{\Moptions nextwarning=no}*/
/* I want the entry in the manual at this point 
extern __exportF void MULMULEY_SEGMENTS(const list<segment>& S, 
                                        GRAPH<point,segment>& G, 
                                        bool embed=false);
                                        */
/*{\Mfuncl
as above.\\
There is one additional output convention. If $G$ is an undirected graph,
the undirected planar map corresponding to $G(s)$ is computed.
The computation follows the incremental algorithm of Mulmuley (\cite{MU94})
whose expected running time is $O(M + s + n \log n)$, where $n$ is the 
number of segments, $s$ is the number of vertices of the graph $G$,
and $m$ is the number of edges.  }*/

// segment intersection reporting

extern __exportF void BALABAN_SEGMENTS(const list<segment>& S, 
                          void (*report)(const segment&,const segment&));

extern __exportF void TRIVIAL_SEGMENTS(const list<segment>& S, 
                          void (*report)(const segment&,const segment&));

inline void SEGMENT_INTERSECTION(const list<segment>& S,
                               void (*report)(const segment&,const segment&))
{ BALABAN_SEGMENTS(S,report); }
/*{\Mfuncl
takes a list of segments $S$ as input and executes for every
pair $(s_1,s_2)$ of intersecting segments $|report|(s_1,s_2)$. 
The algorithm (\cite{BA95}) has running time $O(n log^2 n + k)$, 
where $n$ is the number of segments and $k$ is the number intersecting
pairs of segments. }*/

inline void SEGMENT_INTERSECTION(const list<segment>& S, list<point>& P)
{ SWEEP_SEGMENTS(S,P); }
/*{\Mfuncl
takes a list of segments $S$ as input, computes the set of (proper) 
intersection points between all segments in $S$ and stores this set in $P$.
The algorithm (\cite{BO79}) has running time 
$O((\Labs{P}+\Labs{S})\log \Labs{S})$.
}*/


/*{\Mtext
\bigskip
$\bullet$ {\bf Red-Blue Line Segment Intersection}
}*/

extern __exportF void SWEEP_SEGMENTS_RB(const list<segment>& S1, 
                                        const list<segment>& S2, 
                                        GRAPH<point,segment>& G, 
                                        bool embed=false,
                                        bool use_optimization = true);

extern __exportF void SWEEP_SEGMENTS_RB(const list<segment>&, 
                                        const list<segment>&, 
                                        list<point>&);

inline void SEGMENT_INTERSECTION(const list<segment>& S1, 
                                 const list<segment>& S2,
                                 GRAPH<point,segment>& G, 
                                 bool embed=false)
{ SWEEP_SEGMENTS_RB(S1,S2,G,embed,true); }

/*{\Mfuncl takes two lists of segments $S_1$ and $S_2$ as input and computes 
the planar graph $G$ induced by the set of straight line segments
in $S_1\cup S_2$ (as defined above). \precond Any pair of segments 
in $S_1$ or $S_2$, respectively, does not intersect in a point different
from one of the endpoints of the segments, i.e. segments of $S_1$ 
or $S_2$ are either pairwise disjoint or have a common endpoint.
}*/




/*{\Mtext
\bigskip
$\bullet$ {\bf Closest Pairs}
\setopdims{1.3cm}{1.6cm}
}*/

extern __exportF double CLOSEST_PAIR(list<point>& L, point& r1, point& r2);
/*{\Mfuncl
CLOSEST\_PAIR takes as input a list of points $L$. It computes a pair
of points $r1,r2 \in L$ with minimal Euclidean distance and returns the
squared distance between $r1$ and $r2$. 
The algorithm (\cite{PS85}) has running time $O(n\log n)$ 
where $n$ is the number of input points.
}*/



/*{\Mtext
\bigskip
$\bullet$ {\bf Miscellaneous Functions}
}*/

extern __exportF void Bounding_Box(const list<point>& L, point& pl, point& pb,
                                                         point& pr, point& pt);
/*{\Mfuncl
computes four points $pl,pb,pr,pt$ from $L$ such that 
$(xleft,ybot,xright,ytop)$ with |xleft = pl.xcoord()|, |ybot = pb.ycoord()|, 
|xright = pr.xcoord()| and |ytop = pt.ycoord()| is the smallest iso-oriented 
rectangle containing all points of $L$. \precond  $L$ is not empty. }*/


extern __exportF bool Is_Simple_Polygon(const list<point>& L);
/*{\Mfuncl
takes as input a list of points $L$ and returns $true$ if $L$
is the vertex sequence of a simple polygon and false otherwise. 
The algorithms has running time $O(n\log n)$, where $n$ is the
number of points in $L$. }*/

LEDA_END_NAMESPACE

#ifndef LEDA_GEO_CHECK_H
#define LEDA_GEO_CHECK_H

#include <LEDA/graph/graph.h>
#include <LEDA/graph/graph_alg.h>
#include <LEDA/core/d_array.h>
 
LEDA_BEGIN_NAMESPACE

/*{\Mtext
\bigskip
$\bullet$ {\bf Properties of Geometric Graphs}
\medskip


We give procedures to check properties of geometric graph.
We give procedures to verify properties of \emph{geometric graph}.
A geometric graph is a straight-line embedded map. Every node 
is mapped to a point in the plane and every dart is mapped to the line segment
connecting its endpoints. 

We use |geo_graph| as a template parameter for geometric graph. Any
instantiation of |geo_graph| must provide a function

|VECTOR edge_vector(const geo_graph& G, const edge& e)|

that returns a vector  
from the source to the target of $e$. In order to use any of these
template functions the file /LEDA/geo/generic/geo\_check.h must be included.

\settowidth{\typewidth}{|string|}
\addtolength{\typewidth}{2\colsep}
\setlength{\callwidth}{2cm}
\computewidths
}*/

template <class geo_graph>
bool Is_CCW_Ordered(const geo_graph& G);
/*{\Mfunc returns true if for all nodes $v$ the neighbors of $v$ are in
increasing counter-clockwise order around $v$.}*/

template <class geo_graph>
bool Is_CCW_Weakly_Ordered(const geo_graph& G);
/*{\Mfunc returns true if for all nodes $v$ the neighbors of $v$ are in
non-decreasing counter-clockwise order around $v$.}*/

template <class geo_graph>
bool Is_CCW_Ordered_Plane_Map(const geo_graph& G);
/*{\Mfunc Equivalent to |Is_Plane_Map(G)| and |Is_CCW_Ordered(G)|.}*/

template <class geo_graph>
bool Is_CCW_Weakly_Ordered_Plane_Map(const geo_graph& G);
/*{\Mfunc Equivalent to |Is_Plane_Map(G)| and |Is_CCW_Weakly_Ordered(G)|.}*/

template <class geo_graph>
void SORT_EDGES(geo_graph& G);
/*{\Mfunc Reorders the edges of $G$ such that for every node $v$ the 
edges in $A(v)$ are in non-decreasing order by angle.}*/



template <class geo_graph>
bool Is_CCW_Convex_Face_Cycle(const geo_graph& G, const edge& e);
/*{\Mfunc returns true if the face cycle of $G$ containing $e$
defines a counter-clockwise convex polygon, i.e, if the face cycle forms a 
cyclically increasing sequence of edges according to the compare-by-angles
ordering. }*/

template <class geo_graph>
bool Is_CCW_Weakly_Convex_Face_Cycle(const geo_graph& G, const edge& e);
/*{\Mfunc returns true if the face cycle of $G$ containing $e$
defines a counter-clockwise weakly
convex polygon, i.e, if the face cycle forms a 
cyclically non-decreasing sequence of edges according to the compare-by-angles
ordering. }*/

template <class geo_graph>
bool Is_CW_Convex_Face_Cycle(const geo_graph& G, const edge& e);
/*{\Mfunc returns true if the face cycle of $G$ containing $e$
defines a clockwise convex polygon, i.e, if the face cycle forms a 
cyclically decreasing sequence of edges according to the compare-by-angles
ordering. }*/

template <class geo_graph>
bool Is_CW_Weakly_Convex_Face_Cycle(const geo_graph& G, const edge& e);
/*{\Mfunc returns true if the face cycle of $G$ containing $e$
defines a clockwise weakly
convex polygon, i.e, if the face cycle forms a 
cyclically non-increasing sequence of edges according to the compare-by-angles
ordering. }*/

LEDA_END_NAMESPACE

#endif


#if LEDA_ROOT_INCL_ID == 500096
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif


#endif

