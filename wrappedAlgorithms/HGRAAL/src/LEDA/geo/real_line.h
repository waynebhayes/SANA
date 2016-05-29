/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  real_line.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.3 $  $Date: 2005/04/14 10:45:00 $

#ifndef LEDA_REAL_LINE_H
#define LEDA_REAL_LINE_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 442002
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/geo/line.h>
#include <LEDA/geo/real_point.h>
#include <LEDA/geo/real_segment.h>
#include <LEDA/geo/real_ray.h>

LEDA_BEGIN_NAMESPACE

//------------------------------------------------------------------------------
// real straight lines
//------------------------------------------------------------------------------

class __exportC real_line;

class __exportC real_line_rep : public handle_rep {

friend class __exportC real_line;
friend class __exportC real_circle;

  real_segment  seg; 

public:
   
  real_line_rep() {}
  real_line_rep(const real_segment& s) : seg(s) {}

 ~real_line_rep() {}

friend inline int cmp_slopes(const real_line&, const real_line&);
friend inline int orientation(const real_line&, const real_point&);

};
   
/*{\Manpage {real_line} {} {Straight Real Lines} {l}}*/

class __exportC rat_line;

class __exportC real_line   : public HANDLE_BASE(real_line_rep) 
{

friend class __exportC real_circle;


/*{\Mdefinition
An instance $l$ of the data type |\Mname| is a directed straight line
in the two-dimensional plane.}*/

real_line_rep* ptr() const { return (real_line_rep*)PTR; }

public:

/*{\Mtypes 5}*/ 

typedef real       coord_type;
/*{\Mtypemember the coordinate type (|real|).}*/

typedef real_point point_type;
/*{\Mtypemember the point type (|real_point|).}*/

typedef line       float_type;



/*{\Mcreation}*/

 real_line(const real_point& p, const real_point& q);
/*{\Mcreate 
introduces a variable |\Mvar| of type |\Mname|. |\Mvar| is initialized to the 
line passing through points $p$ and $q$ directed form $p$ to $q$.}*/


 real_line(const real_segment& s);
/*{\Mcreate 
introduces a variable |\Mvar| of type |\Mname|. |\Mvar| is initialized to the 
line supporting segment $s$.}*/

 real_line(const real_ray& r);
/*{\Mcreate 
introduces a variable |\Mvar| of type |\Mname|. |\Mvar| is initialized to the 
line supporting ray $r$.}*/

 real_line(const real_point& p, const real_vector& v);
/*{\Mcreate 
introduces a variable |\Mvar| of type |\Mname|. |\Mvar| is initialized to the 
line passing through points $p$ and $p+v$.}*/

 real_line();
/*{\Mcreate 
introduces a variable |\Mvar| of type |\Mname|. |\Mvar| is initialized to the 
line passing through the origin with direction 0.}*/


 real_line(const line& l1, int prec = 0)
{ PTR = new real_line_rep(real_segment(real_point(l1.point1(),prec), 
                                       real_point(l1.point2(),prec))); }
/*{\Mcreate 
introduces a variable |\Mvar| of type |\Mname| initialized
to the line $l_1$. 
(The second argument is for compatibility with |rat_line|.)}*/

 real_line(const rat_line& l1);
/*{\Mcreate 
introduces a variable |\Mvar| of type |\Mname| initialized
to the line $l_1$.}*/

 real_line(const real_line& l) : HANDLE_BASE(real_line_rep)(l) {};
 real_line& operator=(const real_line& l) { HANDLE_BASE(real_line_rep)::operator=(l); return *this; }
~real_line() {}



/*{\Moperations 2 5.0 }*/

real_point   point1() const { return ptr()->seg.source(); }
/*{\Mop     returns a point on |\Mvar|.}*/

real_point   point2() const { return ptr()->seg.target(); }
/*{\Mop     returns a second point on |\Mvar|.}*/


real_segment seg()   const  { return ptr()->seg; }
/*{\Mop     returns a segment on |\Mvar|.}*/




line   to_float() const { return to_line(); }
line   to_line() const { return line(point1().to_point(), point2().to_point()); }
/*{\Xop  for compatibility with rat_line. }*/


void    normalize() const {}
/*{\Xop  for compatibility with |rat_segment|. }*/



bool is_vertical() const    { return ptr()->seg.is_vertical();  }
/*{\Mop     returns true iff |\Mvar| is vertical.}*/

bool is_horizontal() const  { return ptr()->seg.is_horizontal();}
/*{\Mop     returns true iff |\Mvar| is horizontal.}*/


real sqr_dist(const real_point& q) const;
/*{\Mop     returns the square of the distance between |\Mvar| and |q|.}*/

real distance(const real_point& q) const;
/*{\Mop     returns the distance between |\Mvar| and |q|.}*/

int orientation(const real_point& p) const { return ptr()->seg.orientation(p); }
/*{\Mop     returns $orientation(\Mvar.point1(),\Mvar.point2(),p)$.}*/


real slope() const     { return ptr()->seg.slope();     }
/*{\Mop     returns the slope of |\Mvar|.\\
	    \precond  |\Mvar|  is not vertical.}*/

real y_proj(real x) const  { return ptr()->seg.y_proj(x); };
/*{\Mop     returns $p$.ycoord(), where $p \in l$ with $p$.xcoord() = $x$.\\
            \precond |\Mvar| is not vertical.}*/

real x_proj(real y) const  { return ptr()->seg.x_proj(y); };
/*{\Mop     returns $p$.xcoord(), where $p \in l$ with $p$.ycoord() = $y$.\\
            \precond |\Mvar| is not horizontal.}*/

real y_abs() const { return ptr()->seg.y_proj(0); }
/*{\Mop     returns the y-abscissa of |\Mvar| (|\Mvar|.y\_proj(0)).\\
	    \precond  |\Mvar|  is not vertical.}*/

bool intersection(const real_line& g, real_point& p) const;
/*{\Mopl    if |\Mvar| and $g$ intersect in a single point this point
            is assigned to $p$ and the result is true, otherwise 
            the result is false. }*/

bool intersection(const real_segment& s, real_point& inter) const;
/*{\Mopl    if |\Mvar| and $s$ intersect in a single point this point
            is assigned to $p$ and the result is true, otherwise 
            the result is false. }*/
	    
bool intersection(const real_segment& s) const;
/*{\Mopl    returns |true|, if |\Mvar| and $s$ intersect, |false| otherwise.}*/

real_line translate(real dx, real dy) const 
{ return ptr()->seg.translate(dx,dy); }
/*{\Mopl     returns |\Mvar| translated by vector $(dx,dy)$.}*/

real_line translate(const real_vector& v)  const 
{ return ptr()->seg.translate(v); }
/*{\Mopl     returns |\Mvar| translated by vector $v$.\\
	    \precond $v$.dim() = 2.}*/ 

real_line operator+(const real_vector& v) const { return translate(v); }
/*{\Mbinop  returns |\Mvar| translated by vector $v$.}*/

real_line operator-(const real_vector& v) const { return translate(-v); }
/*{\Mbinop  returns |\Mvar| translated by vector $-v$.}*/


real_line rotate90(const real_point& q, int i=1) const
{ return ptr()->seg.rotate90(q,i); }
/*{\Mopl    returns |\Mvar| rotated about $q$ by an angle of $i\times 90$ 
            degrees. If $i > 0$ the rotation is counter-clockwise otherwise
            it is clockwise. }*/


real_line reflect(const real_point& p, const real_point& q) const
{ return ptr()->seg.reflect(p,q); }
/*{\Mop     returns |\Mvar| reflected  across the straight line passing
            through $p$ and $q$.}*/


real_line reverse() const { return ptr()->seg.reverse(); }
/*{\Mop     returns |\Mvar| reversed.}*/


real_segment perpendicular(const real_point& p) const;
/*{\Mop    returns the segment perpendicular to |\Mvar| with source $p$.
           and target on |\Mvar|.}*/

real_point dual() const;
/*{\Mop    returns the point dual to |\Mvar|.\\
           \precond |\Mvar| is not vertical.}*/

inline int side_of(const real_point& p) const { return orientation(p); }
/*{\Mop     computes orientation($a$, $b$, $p$), where $a \not= b$
            and $a$ and $b$ appear in this order on line $l$.}*/


bool contains(const real_point& p) const;
/*{\Mop     returns true if $p$ lies on |\Mvar|.}*/

bool contains(const real_segment&) const;

bool clip(real_point p, real_point q, real_segment& s) const;
/*{\Mop clips $l$ at the rectangle $R$ defined by $p$ and $q$.
Returns true if the intersection of $R$ and |\Mvar| is non-empty
and returns false otherwise. If the intersection is non-empty the 
intersection is assigned to $s$; it is guaranteed that the source node of
$s$ is no larger than its target node. }*/


bool operator==(const real_line& g) const { return contains(g.ptr()->seg); }

bool operator!=(const real_line& g) const { return !contains(g.ptr()->seg); }


/*{\Mtext
{\bf Non-Member Functions}
\smallskip
}*/


friend int orientation(const real_line& l, const real_point& p);
/*{\Mfunc      computes orientation($a$, $b$, $p$) (see the manual
page of |real_point|), where $a \not= b$
and $a$ and $b$ appear in this order on line $l$. }*/

friend int cmp_slopes(const real_line& l1, const real_line& l2);
/*{\Mfunc      returns compare(slope($l_1$), slope($l_2$)).}*/

friend __exportF istream& operator>>(istream& in, real_line& l);  

};

inline ostream& operator<<(ostream& out, const real_line& l) 
{ return out << l.seg(); }

inline  int orientation(const real_line& l, const real_point& p)
{ return l.orientation(p); }

inline  int cmp_slopes(const real_line& l1, const real_line& l2)
{ return cmp_slopes(l1.ptr()->seg,l2.ptr()->seg); }

inline bool parallel(const real_line& l1, const real_line& l2)
{ return cmp_slopes(l1,l2) == 0; }


extern __exportF real_line p_bisector(const real_point& p, const real_point& q);


inline const char* leda_tname(const real_line*) { return "real_line"; }

#if LEDA_ROOT_INCL_ID == 442002
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif


LEDA_END_NAMESPACE

#endif
