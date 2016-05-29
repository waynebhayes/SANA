/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  rat_geo_alg.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:57 $


#ifndef LEDA_RAT_GEO_ALG_H
#define LEDA_RAT_GEO_ALG_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500247
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/system/basic.h>
#include <LEDA/geo/geo_global_enums.h>
#include <LEDA/core/list.h>
#include <LEDA/graph/graph.h>
#include <LEDA/geo/rat_kernel.h>

LEDA_BEGIN_NAMESPACE


extern __exportF list<rat_point> CONVEX_HULL(const list<rat_point>& L);

extern __exportF rat_polygon CONVEX_HULL_POLY(const list<rat_point>& L);

extern __exportF list<rat_point> UPPER_CONVEX_HULL(const list<rat_point>& L);

extern __exportF list<rat_point> LOWER_CONVEX_HULL(const list<rat_point>& L);



extern __exportF list<rat_point> CONVEX_HULL_S(const list<rat_point>& L);

extern __exportF list<rat_point> CONVEX_HULL_IC(const list<rat_point>& L);
extern __exportF list<rat_point> CONVEX_HULL_IC1(const list<rat_point>& L);
extern __exportF list<rat_point> CONVEX_HULL_IC2(const list<rat_point>& L);
extern __exportF list<rat_point> CONVEX_HULL_IC3(const list<rat_point>& L);

extern __exportF list<rat_point> CONVEX_HULL_RIC(const list<rat_point>& L);

extern __exportF rational WIDTH(const list<rat_point>& L, rat_line& l1, rat_line& l2);
  
  
  
extern __exportF void HALFPLANE_INTERSECTION(const list<rat_line>& L, list<rat_line>& Lout);

extern __exportF void HALFPLANE_INTERSECTION(const list<rat_line>& L, list<rat_segment>& S);




//#include <LEDA/geo/geo_global_enums.h>

extern __exportF edge TRIANGULATE_POINTS(const list<rat_point>&,GRAPH<rat_point,int>&);

extern __exportF int DELAUNAY_FLIPPING(GRAPH<rat_point,int>& T,
                        delaunay_voronoi_kind k = NEAREST);
extern __exportF int DELAUNAY_FLIPPING(GRAPH<rat_point,int>& T, list<edge> L,
                        delaunay_voronoi_kind k = NEAREST);

extern __exportF int DELAUNAY_FLIP(const list<rat_point>&,GRAPH<rat_point,int>&);
extern __exportF int F_DELAUNAY_FLIP(const list<rat_point>&,GRAPH<rat_point,int>&);

extern __exportF void DELAUNAY_STOLFI(const list<rat_point>&,GRAPH<rat_point,int>&,bool=false);
extern __exportF void DELAUNAY_DWYER (const list<rat_point>&,GRAPH<rat_point,int>&,bool=false);



extern __exportF int  DELAUNAY_FLIP_F(const list<rat_point>&,GRAPH<rat_point,int>&);
extern __exportF void DELAUNAY_STOLFI_F(const list<rat_point>&,GRAPH<rat_point,int>&,bool=false);
extern __exportF void DELAUNAY_DWYER_F(const list<rat_point>&,GRAPH<rat_point,int>&,bool=false);


extern __exportF void DELAUNAY_TRIANG(const list<rat_point>& L,GRAPH<rat_point,int>& G);
extern __exportF void DELAUNAY_DIAGRAM(const list<rat_point>& L,GRAPH<rat_point,int>& G);


extern __exportF void F_DELAUNAY_TRIANG(const list<rat_point>& L, GRAPH<rat_point,int>& G);
extern __exportF void F_DELAUNAY_DIAGRAM(const list<rat_point>& L, GRAPH<rat_point,int>& G);


// constrained triangulations

extern __exportF edge TRIANGULATE_SEGMENTS(const list<rat_segment>& L, 
                                    GRAPH<rat_point,rat_segment>& G);


extern __exportF edge TRIANGULATE_PLANE_MAP(GRAPH<rat_point,rat_segment>& G);

extern __exportF edge DELAUNAY_TRIANG(const list<rat_segment>& L, 
                                    GRAPH<rat_point,rat_segment>& G);

extern __exportF edge DELAUNAY_TRIANG(GRAPH<rat_point,rat_segment>& G);




extern __exportF edge TRIANGULATE_POLYGON(const rat_polygon& P, 
                                   GRAPH<rat_point,rat_segment>& G,
                                   list<edge>& inner_edges,
                                   list<edge>& outer_edges,
                                   list<edge>& boundary_edges);


extern __exportF edge TRIANGULATE_POLYGON(const rat_gen_polygon& gpoly, 
                                       GRAPH<rat_point,rat_segment>& G, 
                                       list<edge>& inner_edges,
                                       list<edge>& outer_edges,
                                       list<edge>& boundary_edges,
                                       list<edge>& hole_edges);	



extern __exportF edge CONVEX_COMPONENTS(const rat_polygon& P,
                                   GRAPH<rat_point, rat_segment>& G,
                                   list<edge>& inner_edges,
                                   list<edge>& boundary_edges);

extern __exportF edge CONVEX_COMPONENTS(const rat_gen_polygon& GP, 
                                 GRAPH<rat_point,rat_segment>& G,
                                 list<edge>& inner_edges,
                                 list<edge>& boundary_edges,
                                 list<edge>& hole_edges);


extern __exportF list<rat_polygon> TRIANGLE_COMPONENTS(const rat_gen_polygon& GP);

extern __exportF list<rat_polygon> CONVEX_COMPONENTS(const rat_gen_polygon& GP);

/*
inline list<polygon> TRIANGLE_COMPONENTS(const polygon& P)
{ return TRIANGLE_COMPONENTS(gen_polygon(P)); }

inline list<polygon> CONVEX_COMPONENTS(const polygon& P)
{ return CONVEX_COMPONENTS(gen_polygon(P)); }
*/



extern __exportF int DELAUNAY_FLIPPING(GRAPH<rat_point,rat_segment>&);





extern __exportF rat_gen_polygon MINKOWSKI_SUM(const rat_polygon& P, 
                                               const rat_polygon& R);


extern __exportF rat_gen_polygon MINKOWSKI_DIFF(const rat_polygon& P, 
                                                const rat_polygon& R);


extern __exportF rat_gen_polygon MINKOWSKI_SUM(const rat_gen_polygon& P, 
                                               const rat_polygon& R);

extern __exportF rat_gen_polygon MINKOWSKI_DIFF(const rat_gen_polygon& P, 
                                                const rat_polygon& R);


// new versions of Minkowski sums/diffs

extern __exportF rat_gen_polygon MINKOWSKI_SUM(const rat_polygon& P, 
                                               const rat_polygon& R,
                        void (*convex_partition)(const rat_gen_polygon&, 
                                                 const rat_polygon&, 
                                                 list<rat_polygon>& , 
                                                 list<rat_polygon>& ) ,
			rat_gen_polygon (*conv_unite)(const list<rat_gen_polygon>&) );					      


extern __exportF rat_gen_polygon MINKOWSKI_DIFF(const rat_polygon& P, 
                                                const rat_polygon& R,
                        void (*convex_partition)(const rat_gen_polygon&, 
                                                 const rat_polygon&, 
                                                 list<rat_polygon>& , 
                                                 list<rat_polygon>& ) ,
			rat_gen_polygon (*conv_unite)(const list<rat_gen_polygon>&) );	


extern __exportF rat_gen_polygon MINKOWSKI_SUM(const rat_gen_polygon& P, 
                                               const rat_polygon& R,
                        void (*convex_partition)(const rat_gen_polygon&, 
                                                 const rat_polygon&, 
                                                 list<rat_polygon>& , 
                                                 list<rat_polygon>& ) ,
			rat_gen_polygon (*conv_unite)(const list<rat_gen_polygon>&) );	

extern __exportF rat_gen_polygon MINKOWSKI_DIFF(const rat_gen_polygon& P, 
                                                const rat_polygon& R,
                        void (*convex_partition)(const rat_gen_polygon&, 
                                                 const rat_polygon&, 
                                                 list<rat_polygon>& , 
                                                 list<rat_polygon>& ) ,
			rat_gen_polygon (*conv_unite)(const list<rat_gen_polygon>&) );	



extern __exportF void MIN_SPANNING_TREE_OLD(const list<rat_point>& L, GRAPH<rat_point,int>& T);

extern __exportF void MIN_SPANNING_TREE(const list<rat_point>& L, GRAPH<rat_point,int>& T);





extern __exportF bool Is_Convex_Subdivision(const GRAPH<rat_point,int>& G);

extern __exportF bool Is_Triangulation(const GRAPH<rat_point,int>& G);






extern __exportF bool Is_Delaunay_Triangulation(const GRAPH<rat_point,int>& G, 
                               delaunay_voronoi_kind kind);

extern __exportF bool Is_Delaunay_Diagram(const GRAPH<rat_point,int>& G, 
                         delaunay_voronoi_kind kind);



extern __exportF void DELAUNAY_TO_VORONOI(const GRAPH<rat_point,int>& DT, 
                                   GRAPH<rat_circle,rat_point>& VD);

extern __exportF void VORONOI(const list<rat_point>& L, GRAPH<rat_circle,rat_point>& VD);

extern __exportF void F_DELAUNAY_TO_VORONOI(const GRAPH<rat_point,int>& DT, 
                                   GRAPH<rat_circle,rat_point>& VD);

extern __exportF void F_VORONOI(const list<rat_point>& L, GRAPH<rat_circle,rat_point>& VD);


extern __exportF rat_circle LARGEST_EMPTY_CIRCLE     (const list<rat_point>& L);
extern __exportF rat_circle SMALLEST_ENCLOSING_CIRCLE(const list<rat_point>& L);

extern __exportF void ALL_EMPTY_CIRCLES (const list<rat_point>&, list<rat_circle>&);
extern __exportF void ALL_ENCLOSING_CIRCLES(const list<rat_point>&, list<rat_circle>&);

extern __exportF bool MIN_AREA_ANNULUS(const list<rat_point>& L, 
                      rat_point& center, 
                      rat_point& ipoint, 
                      rat_point& opoint,
                      rat_line&  l1);


extern __exportF bool MIN_WIDTH_ANNULUS(const list<rat_point>& L, 
                       rat_point& center, 
                       rat_point& ipoint,
                       rat_point& opoint,
                       rat_line& l1,
                       rat_line& l2);


inline bool MIN_AREA_ANNULUS(const list<rat_point>& L, 
                             rat_point& center, 
                             rat_point& ipoint, 
                             rat_point& opoint)
{ rat_line l1;
  return MIN_AREA_ANNULUS(L,center,ipoint,opoint,l1);
 }


inline bool MIN_WIDTH_ANNULUS(const list<rat_point>& L, 
                              rat_point& center, 
                              rat_point& ipoint, 
                              rat_point& opoint)
{ rat_line l1,l2;
  return MIN_WIDTH_ANNULUS(L,center,ipoint,opoint,l1,l2);
 }



extern __exportF void  CRUST(const list<rat_point>& L0, GRAPH<rat_point,int>& G);






bool Is_Voronoi_Diagram(const GRAPH<rat_circle,rat_point>& G, 
                        delaunay_voronoi_kind kind);



extern __exportF void SWEEP_SEGMENTS(const list<rat_segment>&, 
                                     GRAPH<rat_point,rat_segment>&, 
                                     bool=false,bool=true);

extern __exportF void SWEEP_SEGMENTS(const list<rat_segment>&, list<rat_point>&);


extern __exportF void SWEEP_SEGMENTS_RB(const list<rat_segment>&, 
                                        const list<rat_segment>&, 
                                        GRAPH<rat_point,rat_segment>&, 
                                        bool=false,bool=true);

extern __exportF void SWEEP_SEGMENTS_RB(const list<rat_segment>&, 
                                        const list<rat_segment>&, 
                                        list<rat_point>&);




extern __exportF void MULMULEY_SEGMENTS(const list<rat_segment>& S, 
                                        GRAPH<rat_point,rat_segment>& G, 
                                        bool=false);

inline void SEGMENT_INTERSECTION(const list<rat_segment>& S, 
                                 GRAPH<rat_point,rat_segment>& G, 
                                 bool embed=false)
{ MULMULEY_SEGMENTS(S,G,embed); }



extern __exportF void BALABAN_SEGMENTS(const list<rat_segment>& S,
                      void (*rep_func)(const rat_segment&, const rat_segment&));

extern __exportF void TRIVIAL_SEGMENTS(const list<rat_segment>& S,
                      void (*rep_func)(const rat_segment&, const rat_segment&));


inline  void SEGMENT_INTERSECTION(const list<rat_segment>& S,
                          void (*report)(const rat_segment&,const rat_segment&))
{ BALABAN_SEGMENTS(S,report); }


inline void SEGMENT_INTERSECTION(const list<rat_segment>& S, list<rat_point>& P)
{ SWEEP_SEGMENTS(S,P); }


// red-blue segment intersection

extern __exportF void SWEEP_SEGMENTS_RB(const list<rat_segment>& S1,
                                        const list<rat_segment>& S2,
                                        GRAPH<point,rat_segment>& G,
                                        bool embed=false,
                                        bool use_optimization = true);

extern __exportF void SWEEP_SEGMENTS_RB(const list<rat_segment>&,
                                        const list<rat_segment>&,
                                        list<point>&);

inline void SEGMENT_INTERSECTION(const list<rat_segment>& S1,  
                                 const list<rat_segment>& S2,
                                 GRAPH<rat_point,rat_segment>& G,
                                 bool embed=false)
{ SWEEP_SEGMENTS_RB(S1,S2,G,embed,true); }




extern __exportF rational CLOSEST_PAIR(list<rat_point>& L, rat_point& r1, rat_point& r2);



extern __exportF void Bounding_Box(const list<rat_point>& L, rat_point& ll, 
                                                             rat_point& ur);

extern __exportF void Bounding_Box(const list<rat_point>& L, rat_point& pl, 
                                                             rat_point& pb,
                                                             rat_point& pr, 
                                                             rat_point& pt);

extern __exportF bool Is_Simple_Polygon(const list<rat_point>& L);


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


#if LEDA_ROOT_INCL_ID == 500247
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif


#endif

