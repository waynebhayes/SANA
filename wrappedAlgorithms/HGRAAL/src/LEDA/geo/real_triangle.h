/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  real_triangle.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:45:01 $

#ifndef LEDA_REAL_TRIANGLE_H
#define LEDA_REAL_TRIANGLE_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 442006
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/geo/triangle.h>
#include <LEDA/geo/real_point.h>
#include <LEDA/geo/real_line.h>
#include <LEDA/geo/geo_global_enums.h>

LEDA_BEGIN_NAMESPACE

class __exportC real_triangle;

//------------------------------------------------------------------------------
// triangles
//------------------------------------------------------------------------------


class __exportC real_triangle_rep : public handle_rep {

static leda_mutex mutex_id_counter;
static unsigned long id_counter;

friend class __exportC real_triangle;
   
   real_point a;
   real_point b;
   real_point c;

   unsigned long id;

public:
   
   real_triangle_rep(const real_point&, const real_point&, const real_point&);
  ~real_triangle_rep() {}

   friend inline unsigned long ID_Number(const real_triangle&);

};

/*{\Manpage {real_triangle} {} {Real Triangles} {t}}*/

class __exportC rat_triangle;

class __exportC real_triangle  : public HANDLE_BASE(real_triangle_rep) 
{
/*{\Mdefinition
    An instance |\Mvar| of the data type |\Mname| is an oriented triangle
    in the two-dimensional plane. A triangle splits the plane into one
    bounded and one unbounded region. If the triangle is positively
    oriented, the bounded region is to the left of it, if it is negatively
    oriented, the unbounded region is to the left of it.
    A triangle |\Mvar| is called degenerate, if the 3 vertices of |\Mvar| are collinear.
}*/

real_triangle_rep* ptr() const { return (real_triangle_rep*)PTR; }

public:

/*{\Mtypes 5}*/ 

typedef real       coord_type;
/*{\Mtypemember the coordinate type (|real|).}*/

typedef real_point point_type;
/*{\Mtypemember the point type (|real_point|).}*/

typedef triangle   float_type;


/*{\Mcreation t}*/

real_triangle();                 
/*{\Mcreate introduces a variable |\Mvar| of type |\Mname|. |\Mvar| is 
            initialized to the empty triangle.}*/


real_triangle(const real_point& p, const real_point& q, const real_point& r); 
/*{\Mcreate introduces a variable |\Mvar| of type |\Mname|. |\Mvar| is 
            initialized to the triangle $[p,q,r]$. }*/


real_triangle(real x1, real y1, real x2, real y2, real x3, real y3) ;
/*{\Mcreate introduces a variable |\Mvar| of type |\Mname|. |\Mvar| is 
            initialized to the triangle $[(x1,y1),(x2,y2),(x3,y3)]$.}*/ 


// for compatibility with rat_triangle:
 real_triangle(const triangle& t1, int prec = 0)
{ PTR = new real_triangle_rep(real_point(t1.point1(),prec), 
                              real_point(t1.point2(),prec), 
                              real_point(t1.point3(),prec)); }
/*{\Mcreate introduces a variable |\Mvar| of type |\Mname| initialized
            to the triangle $t_1$. 
            (The second argument is for compatibility with |rat_triangle|.)}*/

 real_triangle(const rat_triangle& t1);
/*{\Mcreate introduces a variable |\Mvar| of type |\Mname| initialized
            to the triangle $t_1$.}*/

 real_triangle(const real_triangle& t) : HANDLE_BASE(real_triangle_rep)(t) {}     
~real_triangle() {}
 real_triangle& operator=(const real_triangle& t) 
 { HANDLE_BASE(real_triangle_rep)::operator=(t); return *this;}


/*{\Moperations 2 3.5}*/

triangle  to_float() const { return to_triangle(); }
triangle  to_triangle() const 
{ return triangle(point1().to_point(), point2().to_point(), point3().to_point()); }
/*{\Xop  for compatibility with rat_triangle. }*/


void    normalize() const {}
/*{\Xop  for compatibility with |rat_triangle|. }*/



/*{\Moptions nextwarning=no}*/
real_point point1()    const      { return ptr()->a; }
/*{\Mop       returns the first vertex of triangle |\Mvar|.}*/

real_point point2()    const      { return ptr()->b; }
/*{\Mop       returns the second vertex of triangle |\Mvar|.}*/

real_point point3()    const      { return ptr()->c; }
/*{\Mop       returns the third vertex of triangle |\Mvar|.}*/

real_point operator[](int i) const;
/*{\Marrop returns the $i$-th vertex of |\Mvar|. \precond $1\le i\le 3$.}*/

int orientation() const;
/*{\Mop       returns the orientation of |\Mvar|.}*/

real area() const;
/*{\Mop       returns the signed area of |\Mvar| (positive, if $orientation(a,b,c)>0$,
              negative otherwise).}*/

bool is_degenerate() const;
/*{\Mopl    returns true if the vertices of |\Mvar| are collinear. }*/

int side_of(const real_point& p) const;
/*{\Mopl    returns $+1$ if $p$ lies to the left of |\Mvar|, $0$ if $p$ lies on |\Mvar|
            and $-1$ if $p$ lies to the right of |\Mvar|.}*/

region_kind region_of(const real_point& p) const;
/*{\Mopl    returns $BOUNDED\_REGION$ if $p$ lies in the bounded region of |\Mvar|, 
            $ON\_REGION$ if $p$ lies on |\Mvar| and $UNBOUNDED\_REGION$ if $p$ lies in
	    the unbounded region.}*/

bool   inside(const real_point& p) const;
/*{\Mopl    returns true, if $p$ lies to the left of |\Mvar|.}*/

bool   outside(const real_point& p) const;
/*{\Mopl    returns true, if $p$ lies to the right of |\Mvar|.}*/

bool   on_boundary(const real_point& p) const;
/*{\Mopl    decides whether $p$ lies on the boundary of |\Mvar|.}*/

bool   contains(const real_point& p) const;
/*{\Mopl    decides whether |\Mvar| contains $p$. }*/


bool   intersection(const real_line& l) const;
/*{\Mopl    decides whether the bounded region or the boundary of |\Mvar| and $l$ intersect. }*/

bool   intersection(const real_segment& s) const;
/*{\Mopl    decides whether the bounded region or the boundary of |\Mvar| and $s$ intersect. }*/


real_triangle translate(real dx, real dy) const;
/*{\Mopl    returns |\Mvar| translated by vector $(dx,dy)$.}*/

real_triangle translate(const real_vector& v) const;
/*{\Mop     returns $t+v$, i.e.,  |\Mvar| translated by vector $v$.\\
	    \precond $v$.dim() = 2.}*/ 

real_triangle operator+(const real_vector& v) const { return translate(v); }
/*{\Mbinop  returns |\Mvar| translated by vector $v$.}*/

real_triangle operator-(const real_vector& v) const { return translate(-v); }
/*{\Mbinop  returns |\Mvar| translated by vector $-v$.}*/

real_triangle rotate90(const real_point& q, int i=1) const;
/*{\Mopl    returns |\Mvar| rotated about $q$ by an angle of $i\times 90$ 
            degrees. If $i > 0$ the rotation is counter-clockwise otherwise
            it is clockwise. }*/

real_triangle rotate90(int i=1) const;
/*{\Mop     returns |\Mvar|.rotate90(|\Mvar|.source(),i).}*/


real_triangle reflect(const real_point& p, const real_point& q) const;
/*{\Mop     returns |\Mvar| reflected  across the straight line passing
            through $p$ and $q$.}*/

real_triangle reflect(const real_point& p) const;
/*{\Mop     returns |\Mvar| reflected  across point $p$.}*/


real_triangle reverse() const { return real_triangle(point3(),point2(),point1()); }
/*{\Mop     returns |\Mvar| reversed.}*/

bool operator==(const real_triangle& t2) const;

bool operator!=(const real_triangle& t2) const
{ return (!((*this)==t2)); }


friend inline unsigned long ID_Number(const real_triangle&);

};

inline unsigned long ID_Number(const real_triangle& t) { return t.ptr()->id; }

inline const char* leda_tname(const real_triangle*) { return "real_triangle"; }


inline ostream& operator<<(ostream& out, const real_triangle& T)
{ return out << T.point1() << " " << T.point2() << " " << T.point3(); }

inline istream& operator>>(istream& in, real_triangle& T) 
{ real_point a,b,c;
  in >> a >> b >> c;
  T = real_triangle(a,b,c);
  return in; 
}

#if LEDA_ROOT_INCL_ID == 442006
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif


LEDA_END_NAMESPACE

#endif

