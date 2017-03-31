/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  real_ray.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.3 $  $Date: 2005/04/14 10:45:01 $

#ifndef LEDA_REAL_RAY_H
#define LEDA_REAL_RAY_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 442003
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/geo/ray.h>
#include <LEDA/geo/real_point.h>
#include <LEDA/geo/real_segment.h>

LEDA_BEGIN_NAMESPACE

//------------------------------------------------------------------------------
// real straight rays
//------------------------------------------------------------------------------

class __exportC real_ray;

class __exportC real_ray_rep : public handle_rep {

friend class __exportC real_ray;
friend class __exportC real_line;
//friend class __exportC real_circle;

  real_segment  seg; 

public:
   
  real_ray_rep() {}
  real_ray_rep(const real_segment& s) : seg(s) {}

 ~real_ray_rep() {}

friend inline int cmp_slopes(const real_ray&, const real_ray&);
friend inline int orientation(const real_ray&, const real_point&);

};
   
/*{\Manpage {real_ray} {} {Real Rays} {r}}*/

class __exportC rat_ray;

class __exportC real_ray   : public HANDLE_BASE(real_ray_rep) 
{

friend class __exportC real_line;
//friend class __exportC real_circle;

/*{\Mdefinition
An instance |\Mvar| of the data type |\Mname| is a directed straight ray
in the two-dimensional plane.}*/

real_ray_rep* ptr() const { return (real_ray_rep*)PTR; }

public:

/*{\Mtypes 5}*/ 

typedef real       coord_type;
/*{\Mtypemember the coordinate type (|real|).}*/

typedef real_point point_type;
/*{\Mtypemember the point type (|real_point|).}*/

typedef ray        float_type;


/*{\Mcreation}*/

 real_ray(const real_point& p, const real_point& q);
/*{\Mcreate 
introduces a variable |\Mvar| of type |\Mname|. |\Mvar| is initialized to the 
ray starting at point $p$ and passing through point $q$. }*/


 real_ray(const real_segment& s);
/*{\Mcreate 
introduces a variable |\Mvar| of type |\Mname|. |\Mvar| is initialized to 
|\Mname(s.source(),s.target())|. }*/

 real_ray(const real_point& p, const real_vector& v);
/*{\Mcreate 
introduces a variable |\Mvar| of type |\Mname|. |\Mvar| is initialized to
|\Mname(p,p+v)|.}*/

 real_ray();
/*{\Mcreate 
introduces a variable |\Mvar| of type |\Mname|. |\Mvar| is initialized to the 
ray starting at the origin with direction 0.}*/


 real_ray(const ray& r1, int prec = 0)
{ PTR = new real_ray_rep(real_segment(real_point(r1.source(),prec), 
                                      real_point(r1.point2(),prec))); }
/*{\Mcreate 
introduces a variable |\Mvar| of type |\Mname| initialized
to the ray $r_1$. 
(The second argument is for compatibility with |rat_ray|.)
}*/


 real_ray(const rat_ray& r1);
/*{\Mcreate 
introduces a variable |\Mvar| of type |\Mname| initialized
to the ray $r_1$.
}*/


 real_ray(const real_ray& r) : HANDLE_BASE(real_ray_rep)(r) {};
 real_ray& operator=(const real_ray& r) { HANDLE_BASE(real_ray_rep)::operator=(r); return *this; }
~real_ray() {}



/*{\Moperations 2 5.0 }*/

ray  to_float() const { return to_ray(); }
ray  to_ray() const { return ray(source().to_point(), point2().to_point()); }
/*{\Xop  for compatibility with rat_ray. }*/


void    normalize() const {}
/*{\Xop  for compatibility with |rat_segment|. }*/


real_point  source() const   { return ptr()->seg.source(); }
/*{\Mop     returns the source of |\Mvar|.}*/

real_point  point1() const   { return ptr()->seg.source(); }
/*{\Mop     returns the source of |\Mvar|.}*/

real_point  point2() const   { return ptr()->seg.target(); }
/*{\Mop     returns a point on |\Mvar| different from |\Mvar.source()|.}*/

bool is_vertical() const    { return ptr()->seg.is_vertical();  }
/*{\Mop     returns true iff |\Mvar| is vertical.}*/

bool is_horizontal() const  { return ptr()->seg.is_horizontal();}
/*{\Mop     returns true iff |\Mvar| is horizontal.}*/


real slope() const     { return ptr()->seg.slope();     }
/*{\Mop     returns the slope of the straight line underlying |\Mvar|.\\
	    \precond  |\Mvar|  is not vertical.}*/

real_segment seg()  const     { return ptr()->seg; }


bool intersection(const real_ray& s, real_point& inter) const;
/*{\Mopl    if $r$ and $s$ intersect in a single point this point 
            is assigned to $inter$ and the result is |true|, otherwise 
            the result is |false|.}*/

bool intersection(const real_segment& s, real_point& inter) const;
/*{\Mopl    if $r$ and $s$ intersect in a single point this point 
            is assigned to $inter$ and the result is |true|, otherwise 
            the result is |false|.}*/

real_ray translate(real dx, real dy) const 
{ return ptr()->seg.translate(dx,dy); }
/*{\Mopl     returns |\Mvar| translated by vector $(dx,dy)$.}*/

real_ray translate(const real_vector& v)  const 
{ return ptr()->seg.translate(v); }
/*{\Mopl     returns |\Mvar| translated by vector $v$\\
	    \precond $v$.dim() = 2.}*/ 

real_ray  operator+(const real_vector& v) const { return translate(v); }
/*{\Mbinop   returns |\Mvar| translated by vector $v$.}*/

real_ray  operator-(const real_vector& v) const { return translate(-v); }
/*{\Mbinop   returns |\Mvar| translated by vector $-v$.}*/


real_ray  rotate90(const real_point& q, int i=1) const
{ return ptr()->seg.rotate90(q,i); }
/*{\Mopl    returns |\Mvar| rotated about $q$ by an angle of $i\times 90$ 
            degrees. If $i > 0$ the rotation is counter-clockwise otherwise
            it is clockwise. }*/


real_ray  reflect(const real_point& p, const real_point& q) const
{ return ptr()->seg.reflect(p,q); }
/*{\Mop     returns |\Mvar| reflected  across the straight line passing
            through $p$ and $q$.}*/

real_ray  reflect(const real_point& p) const
{ return ptr()->seg.reflect(p); }
/*{\Mop     returns |\Mvar| reflected  across point $p$.}*/


real_ray reverse() const { return ptr()->seg.reverse(); }
/*{\Mop     returns |\Mvar| reversed.}*/


bool contains(const real_point&) const;
/*{\Mopl    decides whether |\Mvar| contains $p$. }*/

bool contains(const real_segment&) const;
/*{\Mopl    decides whether |\Mvar| contains $s$. }*/


bool operator==(const real_ray& s) const;
bool operator!=(const real_ray& s) const { return !operator==(s); }



/*{\Mtext
{\bf Non-Member Functions}
\smallskip
}*/


friend int orientation(const real_ray& r, const real_point& p);
/*{\Mfunc      computes orientation($a$, $b$, $p$) (see the manual page
of |real_point|), where $a \not= b$
and $a$ and $b$ appear in this order on ray $r$. }*/

friend int cmp_slopes(const real_ray& r1, const real_ray& r2);
/*{\Mfunc      returns compare(slope($r_1$), slope($r_2$)) where
              $slope(r_i)$ denotes the slope of the straight line
              underlying $r_i$. }*/


friend __exportF istream& operator>>(istream& in, real_ray& r);  

};

inline ostream& operator<<(ostream& out, const real_ray& r) 
{ return out << r.seg(); }

inline  int orientation(const real_ray& r, const real_point& p)
{ return orientation(r.ptr()->seg,p); }

inline  int cmp_slopes(const real_ray& r1, const real_ray& r2)
{ return cmp_slopes(r1.ptr()->seg,r2.ptr()->seg); }

inline bool parallel(const real_ray& r1, const real_ray& r2)
{ return cmp_slopes(r1,r2) == 0; }



inline const char* leda_tname(const real_ray*) { return "real_ray"; }

#if LEDA_ROOT_INCL_ID == 442003
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif


LEDA_END_NAMESPACE

#endif
