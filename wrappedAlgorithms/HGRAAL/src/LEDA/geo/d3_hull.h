/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  d3_hull.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:51 $

#ifndef LEDA_D3_HULL_H
#define LEDA_D3_HULL_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500040
#include <LEDA/internal/PREAMBLE.h>
#endif


#include <LEDA/geo/d3_rat_point.h>
#include <LEDA/geo/d3_point.h>
#include <LEDA/graph/graph.h>

LEDA_BEGIN_NAMESPACE

/*{\Manpage {d3_hull} {} {3D Convex Hull Algorithms} }*/

/*{\Mtext
\setopdims{1cm}{3cm}
}*/


extern __exportF void D3_HULL0(list<d3_rat_point>& L, GRAPH<d3_rat_point,int>& H);

extern __exportF void D3_HULL0(list<d3_point>& L, GRAPH<d3_point,int>& H);



extern __exportF void D3_HULL(const list<d3_rat_point>& L, GRAPH<d3_rat_point,int>& H, int=-1);

extern __exportF void D3_HULL(const list<d3_point>& L, GRAPH<d3_point,int>& H, int=-1);


extern __exportF void D3_HULL_FLIP(const list<d3_rat_point>& L, GRAPH<d3_rat_point,int>& H, bool filter=false);


extern __exportF void D3_DC_HULL(const list<d3_rat_point>& L0, 
                                 GRAPH<d3_rat_point, int>& H, int n = 16);





inline void CONVEX_HULL(const list<d3_rat_point>& L,GRAPH<d3_rat_point,int>& H)
{ D3_HULL(L,H); }
/*{\Mfuncl
CONVEX\_HULL takes as argument a list of points and returns the (planar 
embedded) surface graph $H$ of the convex hull of $L$. The algorithm is based 
on an incremental space sweep. The running time is $O(n^2)$ in the worst case
and $O(n\log n)$ for most inputs. }*/

extern __exportF bool CHECK_HULL(const GRAPH<d3_rat_point,int>& H);
/*{\Mfuncl a checker for convex hulls. }*/


inline void CONVEX_HULL(const list<d3_point>& L,GRAPH<d3_point,int>& H)
{ D3_HULL(L,H); }
/*{\Mfuncl a floating point version of |CONVEX_HULL|. }*/

extern __exportF bool CHECK_HULL(const GRAPH<d3_point,int>& H);
/*{\Mfuncl a checker for floating-point convex hulls. }*/

#if LEDA_ROOT_INCL_ID == 500040
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif


LEDA_END_NAMESPACE

#endif
