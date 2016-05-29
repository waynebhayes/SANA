/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  real_geo_alg.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:45:00 $


#ifndef LEDA_REAL_GEO_ALG_H
#define LEDA_REAL_GEO_ALG_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 442010
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/system/basic.h>
#include <LEDA/geo/geo_global_enums.h>
#include <LEDA/core/list.h>
#include <LEDA/graph/graph.h>
#include <LEDA/geo/real_kernel.h>

LEDA_BEGIN_NAMESPACE

extern __exportF list<real_point> CONVEX_HULL(const list<real_point>& L);

extern __exportF real_polygon CONVEX_HULL_POLY(const list<real_point>& L);

extern __exportF list<real_point> UPPER_CONVEX_HULL(const list<real_point>& L);

extern __exportF list<real_point> LOWER_CONVEX_HULL(const list<real_point>& L);



extern __exportF list<real_point> CONVEX_HULL_S(const list<real_point>& L);

extern __exportF list<real_point> CONVEX_HULL_IC(const list<real_point>& L);
extern __exportF list<real_point> CONVEX_HULL_IC1(const list<real_point>& L);
extern __exportF list<real_point> CONVEX_HULL_IC2(const list<real_point>& L);
extern __exportF list<real_point> CONVEX_HULL_IC3(const list<real_point>& L);

extern __exportF list<real_point> CONVEX_HULL_RIC(const list<real_point>& L);

extern __exportF real WIDTH(const list<real_point>& L, real_line& l1, real_line& l2);
  
  
  
extern __exportF void HALFPLANE_INTERSECTION(const list<real_line>& L, list<real_line>& Lout);

extern __exportF void HALFPLANE_INTERSECTION(const list<real_line>& L, list<real_segment>& S);




//#include <LEDA/geo/geo_global_enums.h>

extern __exportF edge TRIANGULATE_POINTS(const list<real_point>&,GRAPH<real_point,int>&);

extern __exportF int DELAUNAY_FLIPPING(GRAPH<real_point,int>& T,
                        delaunay_voronoi_kind k = NEAREST);
extern __exportF int DELAUNAY_FLIPPING(GRAPH<real_point,int>& T, list<edge> L,
                        delaunay_voronoi_kind k = NEAREST);

extern __exportF int DELAUNAY_FLIP(const list<real_point>&,GRAPH<real_point,int>&);
extern __exportF int F_DELAUNAY_FLIP(const list<real_point>&,GRAPH<real_point,int>&);

extern __exportF void DELAUNAY_STOLFI(const list<real_point>&,GRAPH<real_point,int>&,bool=false);
extern __exportF void DELAUNAY_DWYER (const list<real_point>&,GRAPH<real_point,int>&,bool=false);



extern __exportF int  DELAUNAY_FLIP_F(const list<real_point>&,GRAPH<real_point,int>&);
extern __exportF void DELAUNAY_STOLFI_F(const list<real_point>&,GRAPH<real_point,int>&,bool=false);
extern __exportF void DELAUNAY_DWYER_F(const list<real_point>&,GRAPH<real_point,int>&,bool=false);


extern __exportF void DELAUNAY_TRIANG(const list<real_point>& L,GRAPH<real_point,int>& G);
extern __exportF void DELAUNAY_DIAGRAM(const list<real_point>& L,GRAPH<real_point,int>& G);


extern __exportF void F_DELAUNAY_TRIANG(const list<real_point>& L, GRAPH<real_point,int>& G);
extern __exportF void F_DELAUNAY_DIAGRAM(const list<real_point>& L, GRAPH<real_point,int>& G);


// constrained triangulations

extern __exportF edge TRIANGULATE_SEGMENTS(const list<real_segment>& L, 
                                    GRAPH<real_point,real_segment>& G);


extern __exportF edge TRIANGULATE_PLANE_MAP(GRAPH<real_point,real_segment>& G);

extern __exportF edge DELAUNAY_TRIANG(const list<real_segment>& L, 
                                    GRAPH<real_point,real_segment>& G);

extern __exportF edge DELAUNAY_TRIANG(GRAPH<real_point,real_segment>& G);




extern __exportF edge TRIANGULATE_POLYGON(const real_polygon& P, 
                                   GRAPH<real_point,real_segment>& G,
                                   list<edge>& inner_edges,
                                   list<edge>& outer_edges,
                                   list<edge>& boundary_edges);


extern __exportF edge TRIANGULATE_POLYGON(const real_gen_polygon& gpoly, 
                                       GRAPH<real_point,real_segment>& G, 
                                       list<edge>& inner_edges,
                                       list<edge>& outer_edges,
                                       list<edge>& boundary_edges,
                                       list<edge>& hole_edges);	



extern __exportF edge CONVEX_COMPONENTS(const real_polygon& P,
                                   GRAPH<real_point, real_segment>& G,
                                   list<edge>& inner_edges,
                                   list<edge>& boundary_edges);

extern __exportF edge CONVEX_COMPONENTS(const real_gen_polygon& GP, 
                                 GRAPH<real_point,real_segment>& G,
                                 list<edge>& inner_edges,
                                 list<edge>& boundary_edges,
                                 list<edge>& hole_edges);


extern __exportF list<real_polygon> TRIANGLE_COMPONENTS(const real_gen_polygon& GP);

extern __exportF list<real_polygon> CONVEX_COMPONENTS(const real_gen_polygon& GP);

/*
inline list<polygon> TRIANGLE_COMPONENTS(const polygon& P)
{ return TRIANGLE_COMPONENTS(gen_polygon(P)); }

inline list<polygon> CONVEX_COMPONENTS(const polygon& P)
{ return CONVEX_COMPONENTS(gen_polygon(P)); }
*/



extern __exportF int DELAUNAY_FLIPPING(GRAPH<real_point,real_segment>&);





extern __exportF real_gen_polygon MINKOWSKI_SUM(const real_polygon& P, 
                                               const real_polygon& R);


extern __exportF real_gen_polygon MINKOWSKI_DIFF(const real_polygon& P, 
                                                const real_polygon& R);


extern __exportF real_gen_polygon MINKOWSKI_SUM(const real_gen_polygon& P, 
                                               const real_polygon& R);

extern __exportF real_gen_polygon MINKOWSKI_DIFF(const real_gen_polygon& P, 
                                                const real_polygon& R);


// new versions of Minkowski sums/diffs

extern __exportF real_gen_polygon MINKOWSKI_SUM(const real_polygon& P, 
                                               const real_polygon& R,
                        void (*convex_partition)(const real_gen_polygon&, 
                                                 const real_polygon&, 
                                                 list<real_polygon>& , 
                                                 list<real_polygon>& ) ,
			real_gen_polygon (*conv_unite)(const list<real_gen_polygon>&) );					      


extern __exportF real_gen_polygon MINKOWSKI_DIFF(const real_polygon& P, 
                                                const real_polygon& R,
                        void (*convex_partition)(const real_gen_polygon&, 
                                                 const real_polygon&, 
                                                 list<real_polygon>& , 
                                                 list<real_polygon>& ) ,
			real_gen_polygon (*conv_unite)(const list<real_gen_polygon>&) );	


extern __exportF real_gen_polygon MINKOWSKI_SUM(const real_gen_polygon& P, 
                                               const real_polygon& R,
                        void (*convex_partition)(const real_gen_polygon&, 
                                                 const real_polygon&, 
                                                 list<real_polygon>& , 
                                                 list<real_polygon>& ) ,
			real_gen_polygon (*conv_unite)(const list<real_gen_polygon>&) );	

extern __exportF real_gen_polygon MINKOWSKI_DIFF(const real_gen_polygon& P, 
                                                const real_polygon& R,
                        void (*convex_partition)(const real_gen_polygon&, 
                                                 const real_polygon&, 
                                                 list<real_polygon>& , 
                                                 list<real_polygon>& ) ,
			real_gen_polygon (*conv_unite)(const list<real_gen_polygon>&) );	



extern __exportF void MIN_SPANNING_TREE_OLD(const list<real_point>& L, GRAPH<real_point,int>& T);

extern __exportF void MIN_SPANNING_TREE(const list<real_point>& L, GRAPH<real_point,int>& T);





extern __exportF bool Is_Convex_Subdivision(const GRAPH<real_point,int>& G);

extern __exportF bool Is_Triangulation(const GRAPH<real_point,int>& G);






extern __exportF bool Is_Delaunay_Triangulation(const GRAPH<real_point,int>& G, 
                               delaunay_voronoi_kind kind);

extern __exportF bool Is_Delaunay_Diagram(const GRAPH<real_point,int>& G, 
                         delaunay_voronoi_kind kind);



extern __exportF void DELAUNAY_TO_VORONOI(const GRAPH<real_point,int>& DT, 
                                   GRAPH<real_circle,real_point>& VD);

extern __exportF void VORONOI(const list<real_point>& L, GRAPH<real_circle,real_point>& VD);

extern __exportF void F_DELAUNAY_TO_VORONOI(const GRAPH<real_point,int>& DT, 
                                   GRAPH<real_circle,real_point>& VD);

extern __exportF void F_VORONOI(const list<real_point>& L, GRAPH<real_circle,real_point>& VD);


extern __exportF real_circle LARGEST_EMPTY_CIRCLE     (const list<real_point>& L);
extern __exportF real_circle SMALLEST_ENCLOSING_CIRCLE(const list<real_point>& L);

extern __exportF void ALL_EMPTY_CIRCLES (const list<real_point>&, list<real_circle>&);
extern __exportF void ALL_ENCLOSING_CIRCLES(const list<real_point>&, list<real_circle>&);

extern __exportF bool MIN_AREA_ANNULUS(const list<real_point>& L, 
                      real_point& center, 
                      real_point& ipoint, 
                      real_point& opoint,
                      real_line&  l1);


extern __exportF bool MIN_WIDTH_ANNULUS(const list<real_point>& L, 
                       real_point& center, 
                       real_point& ipoint,
                       real_point& opoint,
                       real_line& l1,
                       real_line& l2);


inline bool MIN_AREA_ANNULUS(const list<real_point>& L, 
                             real_point& center, 
                             real_point& ipoint, 
                             real_point& opoint)
{ real_line l1;
  return MIN_AREA_ANNULUS(L,center,ipoint,opoint,l1);
 }


inline bool MIN_WIDTH_ANNULUS(const list<real_point>& L, 
                              real_point& center, 
                              real_point& ipoint, 
                              real_point& opoint)
{ real_line l1,l2;
  return MIN_WIDTH_ANNULUS(L,center,ipoint,opoint,l1,l2);
 }



extern __exportF void  CRUST(const list<real_point>& L0, GRAPH<real_point,int>& G);






bool Is_Voronoi_Diagram(const GRAPH<real_circle,real_point>& G, 
                        delaunay_voronoi_kind kind);



extern __exportF void SWEEP_SEGMENTS(const list<real_segment>&, 
                                     GRAPH<real_point,real_segment>&, 
                                     bool=false,bool=true);

extern __exportF void SWEEP_SEGMENTS(const list<real_segment>&, list<real_point>&);


extern __exportF void SWEEP_SEGMENTS_RB(const list<real_segment>&, 
                                        const list<real_segment>&, 
                                        GRAPH<real_point,real_segment>&, 
                                        bool=false,bool=true);

extern __exportF void SWEEP_SEGMENTS_RB(const list<real_segment>&, 
                                        const list<real_segment>&, 
                                        list<real_point>&);




extern __exportF void MULMULEY_SEGMENTS(const list<real_segment>& S, 
                                        GRAPH<real_point,real_segment>& G, 
                                        bool=false);

inline void SEGMENT_INTERSECTION(const list<real_segment>& S, 
                                 GRAPH<real_point,real_segment>& G, 
                                 bool embed=false)
{ MULMULEY_SEGMENTS(S,G,embed); }



extern __exportF void BALABAN_SEGMENTS(const list<real_segment>& S,
                      void (*rep_func)(const real_segment&, const real_segment&));

extern __exportF void TRIVIAL_SEGMENTS(const list<real_segment>& S,
                      void (*rep_func)(const real_segment&, const real_segment&));


inline  void SEGMENT_INTERSECTION(const list<real_segment>& S,
                          void (*report)(const real_segment&,const real_segment&))
{ BALABAN_SEGMENTS(S,report); }


inline void SEGMENT_INTERSECTION(const list<real_segment>& S, list<real_point>& P)
{ SWEEP_SEGMENTS(S,P); }


// red-blue segment intersection

extern __exportF void SWEEP_SEGMENTS_RB(const list<real_segment>& S1,
                                        const list<real_segment>& S2,
                                        GRAPH<point,real_segment>& G,
                                        bool embed=false,
                                        bool use_optimization = true);

extern __exportF void SWEEP_SEGMENTS_RB(const list<real_segment>&,
                                        const list<real_segment>&,
                                        list<point>&);

inline void SEGMENT_INTERSECTION(const list<real_segment>& S1,  
                                 const list<real_segment>& S2,
                                 GRAPH<real_point,real_segment>& G,
                                 bool embed=false)
{ SWEEP_SEGMENTS_RB(S1,S2,G,embed,true); }




extern __exportF real CLOSEST_PAIR(list<real_point>& L, real_point& r1, real_point& r2);



extern __exportF void Bounding_Box(const list<real_point>& L, real_point& ll, 
                                                             real_point& ur);

extern __exportF void Bounding_Box(const list<real_point>& L, real_point& pl, 
                                                             real_point& pb,
                                                             real_point& pr, 
                                                             real_point& pt);

extern __exportF bool Is_Simple_Polygon(const list<real_point>& L);


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


#if LEDA_ROOT_INCL_ID == 442010
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif


#endif

