/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  real_window.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:45:21 $

#ifndef LEDA_REAL_WINDOW_H
#define LEDA_REAL_WINDOW_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 442015
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/graphics/window.h>

LEDA_BEGIN_NAMESPACE

class __exportC real_point;
class __exportC real_segment;
class __exportC real_ray;
class __exportC real_line;
class __exportC real_circle;
class __exportC real_polygon;
class __exportC real_gen_polygon;
class __exportC real_rectangle;
class __exportC real_triangle;

class __exportC r_circle_segment;
class __exportC r_circle_polygon;
class __exportC r_circle_gen_polygon;


extern __exportF window& operator>>(window& W, real_point& p);
/*{\Mbinopfunc   reads a point $p$: clicking the left button 
             assigns the current cursor position to $p$.}*/

extern __exportF window& operator<<(window& W, const real_point& p);
/*{\Mbinopfunc   diplays point $p$ at window $W$.}*/



extern __exportF window& operator>>(window& W, real_segment& s);
/*{\Mbinopfunc   reads a segment $s$: use the left button to input 
                 the start and end point of $s$.}*/

extern __exportF window& operator<<(window& W, const real_segment& s);
/*{\Mbinopfunc   diplays segment $s$ at window $W$.}*/



extern __exportF window& operator>>(window& W, real_ray& r);
/*{\Mbinopfunc   reads a ray $r$: use the left button to input 
                 the start point and a second point on $r$.}*/

extern __exportF window& operator<<(window& W, const real_ray& r);
/*{\Mbinopfunc   diplays ray $r$ at window $W$.}*/


extern __exportF window& operator>>(window& W, real_line& l);
/*{\Mbinopfunc   reads a line $l$: use the left button to input 
                 two different points on $l$.}*/

extern __exportF window& operator<<(window& W, const real_line& l);
/*{\Mbinopfunc   diplays line $l$ at window $W$.}*/

extern __exportF window& operator>>(window& W, real_circle& C);
/*{\Mbinopfunc   reads a circle $C$: use the left button to input 
                 the center of $C$ and a point on $C$.}*/

extern __exportF window& operator<<(window& W, const real_circle& c);
/*{\Mbinopfunc   diplays circle $c$ at window $W$.}*/


extern __exportF window& operator>>(window& W, real_polygon& P);
/*{\Mbinopfunc   reads a polygon $P$: use the left button to input 
                 the sequence of vertices of $P$, end the sequence 
                 by clicking the right button.}*/


extern __exportF window& operator<<(window& W, const real_polygon& p);
/*{\Mbinopfunc   diplays polygon $p$ at window $W$.}*/


extern __exportF window& operator>>(window& W, real_gen_polygon& P);
/*{\Mbinopfunc   reads a generalized polygon $P$: 
                 use the left button to input 
                 the sequence of vertices of $P$, end the sequence 
                 by clicking the right button.}*/

extern __exportF window& operator<<(window& W, const real_gen_polygon& p);
/*{\Mbinopfunc   diplays generalized polygon $p$ at window $W$.}*/


extern __exportF window& operator>>(window& W, real_rectangle& r);
/*{\Mbinopfunc   reads a rectangle $r$: use the left button to input 
                 the lower left and upper right corner.}*/

extern __exportF window& operator<<(window& W, const real_rectangle& r);
/*{\Mbinopfunc   diplays rectangle $r$ at window $W$.}*/


extern __exportF window& operator>>(window& W, real_triangle& t);
/*{\Mbinopfunc   reads a triangle $t$: use the left button to input 
                 the corners.}*/

extern __exportF window& operator<<(window& W, const real_triangle & t);
/*{\Mbinopfunc   diplays triangle $t$ at window $W$.}*/


extern __exportF window& operator>>(window& W, r_circle_segment& cs);
/*{\Mbinopfunc   reads |cs| from $W$: specify source, target and a third point 
                 on the segment with the left mouse button.}*/

extern __exportF window& operator<<(window& W, const r_circle_segment& cs);
/*{\Mbinopfunc   displays |cs| in the window $W$.}*/

extern __exportF window& draw(window& W, const r_circle_segment& cs, color c=window::fgcol);
/*{\Mbinopfunc   displays |p| in the window $W$ with color $c$.}*/


extern __exportF window& operator>>(window& W, r_circle_polygon& p);
/*{\Mbinopfunc   reads a polygon |p|: use the left button to input 
                 the sequence of edges of |p|, end the sequence 
                 by clicking the right button.}*/

extern __exportF window& operator<<(window& W, const r_circle_polygon& p);
/*{\Mbinopfunc   displays |p| in the window $W$.}*/

extern __exportF window& draw(window& W, const r_circle_polygon& p, color c=window::fgcol);
/*{\Mbinopfunc   displays |p| in the window $W$ with color $c$.}*/

extern __exportF window& draw_filled(window& W, const r_circle_polygon& p, color c=window::fgcol);
/*{\Mbinopfunc   displays |p| in the window $W$ filled with color $c$.}*/


extern __exportF window& operator>>(window& W, r_circle_gen_polygon& p);
/*{\Mbinopfunc   reads a generalized polygon |p|: 
                 use the left button to define the segments, the right 
				 button to close a chain, and the middle button to finish
                 the input.}*/

extern __exportF window& operator<<(window& W, const r_circle_gen_polygon& p);
/*{\Mbinopfunc   displays |p| in the window $W$.}*/

extern __exportF window& draw(window& W, const r_circle_gen_polygon& p, color c=window::fgcol);
/*{\Mbinopfunc   displays |p| in the window $W$ with color $c$.}*/

extern __exportF window& draw_filled(window& W, const r_circle_gen_polygon& p, color c=window::fgcol);
/*{\Mbinopfunc   displays |p| in the window $W$ filled with color $c$.}*/


#if LEDA_ROOT_INCL_ID == 442015
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif


LEDA_END_NAMESPACE

#endif
