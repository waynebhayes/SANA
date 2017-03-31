/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  real_point.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.3 $  $Date: 2005/04/14 10:45:01 $


#ifndef LEDA_REAL_POINT_H
#define LEDA_REAL_POINT_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500280
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/numbers/real.h>
#include <LEDA/geo/point.h>
#include <LEDA/numbers/real_vector.h>
/*
#include <LEDA/core/list.h>
#include <LEDA/core/array.h>
*/

LEDA_BEGIN_NAMESPACE

class __exportC real_point;
//class __exportC real_segment;

//------------------------------------------------------------------------------
// real_points
//------------------------------------------------------------------------------

class __exportC real_point_rep  : public handle_rep {

friend class __exportC real_point;
friend class __exportC real_segment;
// friend class __exportC real_line;
// friend class __exportC real_circle;

static leda_mutex mutex_id_counter;
static unsigned long id_counter;

   real x;
   real y;

   unsigned long id;

public:
   real_point_rep(real = 0, real = 0);
  ~real_point_rep() {}

friend inline unsigned long ID_Number(const real_point&);

};


/*{\Manpage {real_point} {} {Real Points} {p}}*/

class __exportC rat_point;

class __exportC real_point  : public HANDLE_BASE(real_point_rep)
{
/*{\Mdefinition
An instance of the data type |real_point| is a point in the two-dimensional
plane $\real^2$. We use $(x,y)$ to denote a real point with first (or x-)
coordinate $x$ and second (or y-) coordinate $y$.}*/

friend class __exportC real_segment;
// friend class __exportC real_line;
// friend class __exportC real_circle;


real_point_rep* ptr() const { return (real_point_rep*)PTR; }

public:

/*{\Mtypes 5}*/

typedef real       coord_type;
/*{\Mtypemember the coordinate type (|real|).}*/

typedef real_point point_type;
/*{\Mtypemember the point type (|real_point|).}*/

typedef point      float_type;
/*{\Mtypemember the corresponding floating-point type (|point|).}*/  



/*{\Mcreation}*/

 real_point()
{ PTR = new real_point_rep(0,0); }
/*{\Mcreate introduces a variable |\Mvar| of type |\Mname| initialized
            to the point $(0,0)$.}*/

 real_point(real x, real y)
{ PTR = new real_point_rep(x,y); }
/*{\Mcreate introduces a variable |\Mvar| of type |\Mname| initialized to
            the point $(x,y)$.}*/

// for compatibility with rat_point.
 real_point(real x, real y, real w)
{ PTR = new real_point_rep(x/w,y/w); }

 real_point(const real_vector& v) { PTR = new real_point_rep(v[0], v[1]); }
/* {\Mcreate introduces a variable |\Mvar| of type |\Mname| initialized
            to the point $(v[0],v[1])$.\\
            \precond: |v.dim() = 2|. }*/

 real_point(const point& p1, int prec = 0);
/*{\Mcreate introduces a variable |\Mvar| of type |\Mname| initialized
to the point $p_1$. 
(The second argument is for compatibility with |rat_point|.)
}*/

 real_point(const rat_point& p1);
/*{\Mcreate introduces a variable |\Mvar| of type |\Mname| initialized
to the point $p_1$.
}*/

real_point(double x, double y)
{ PTR = new real_point_rep(x,y); }
/*{\Mcreate introduces a variable |\Mvar| of type |\Mname| initialized to
            the real point $(x,y)$.}*/

 real_point(const real_point& p) : HANDLE_BASE(real_point_rep)(p) {}

~real_point() {}

real_point& operator=(const real_point& p)
{ HANDLE_BASE(real_point_rep)::operator=(p); return *this; }


/*{\Moperations 2 4}*/

real  xcoord()  const   { return ptr()->x; }
/*{\Mop     returns the first coordinate of |\Mvar|.}*/

real  ycoord()  const   { return ptr()->y; }
/*{\Mop     returns the second coordinate of |\Mvar|.}*/

point   to_float() const { return to_point(); }
point   to_point() const { return point(xcoord().to_double(),ycoord().to_double()); }
/*{\Xop  for compatibility with |rat_point|. }*/

real_vector  to_vector() const { return real_vector(xcoord(),ycoord()); }
/* {\Mop     returns the vector $\vec{xy}$, where $x$ and $y$ are the current
.}*/

double xcoordD()  const   { return ptr()->x.to_double(); }
/*{\Xop  for compatibility with |rat_point|. }*/

double ycoordD()  const   { return ptr()->y.to_double(); }
/*{\Xop  for compatibility with |rat_point|. }*/

void    normalize() const {}
/*{\Xop  for compatibility with |rat_point|. }*/

real  X()  const   { return ptr()->x; }
/*{\Xop  for compatibility with |rat_point|, same as |xcoord()|. }*/

real  Y()  const   { return ptr()->y; }
/*{\Xop  for compatibility with |rat_point|, same as |ycoord()|. }*/

real  W()  const   { return 1; }
/*{\Xop  for compatibility with |rat_point|, returns the constant 1. }*/


double XD()  const   { return ptr()->x.to_double(); }
/*{\Xop  for compatibility with |rat_point|, same as |xcoord()|. }*/

double YD()  const   { return ptr()->y.to_double(); }
/*{\Xop  for compatibility with |rat_point|, same as |ycoord()|. }*/

double WD()  const   { return 1; }
/*{\Xop  for compatibility with |rat_point|, returns the constant 1. }*/


int     dim() const { return 2; }
/*{\Xop  returns 2.}*/

int orientation(const real_point& q, const real_point& r) const
{ real d1 = (xcoord() - q.xcoord()) * (ycoord() - r.ycoord());
  real d2 = (ycoord() - q.ycoord()) * (xcoord() - r.xcoord());
  return (d1-d2).sign();
}
/*{\Mop     returns $|orientation|(\Mvar,q,r)$ (see below). }*/


real area(const real_point& q, const real_point& r) const
{ return ((xcoord()-q.xcoord()) * (ycoord()-r.ycoord()) -
          (ycoord()-q.ycoord()) * (xcoord()-r.xcoord()))/2; }
/*{\Mop     returns $|area|(\Mvar,q,r)$ (see below). }*/


real  sqr_dist(const real_point& q) const;
/*{\Mop     returns the square of the Euclidean distance between |\Mvar|
            and $q$.}*/

int      cmp_dist(const real_point& q, const real_point& r) const;
/*{\Mopl    returns $|compare|(\Mvar.|sqr_dist|(q),\Mvar.|sqr_dist|(r))$. }*/

real xdist(const real_point& q) const;
/*{\Mopl    returns the horizontal distance between |\Mvar| and $q$. }*/

real ydist(const real_point& q) const;
/*{\Mopl    returns the vertical distance between |\Mvar| and $q$. }*/


real  distance(const real_point& q) const;
/*{\Mop     returns the Euclidean distance between |\Mvar| and $q$.}*/

real  distance() const { return distance(real_point(0,0)); }
/*{\Mop     returns the Euclidean distance between |\Mvar| and $(0,0)$.}*/

real_point   translate(real dx, real dy) const;
/*{\Mopl    returns |\Mvar| translated by vector $(dx,dy)$.}*/

real_point   translate(double dx, double dy) const { return translate(real(dx),real(dy)); }
/*{\Mopl    returns |\Mvar| translated by vector $(dx,dy)$.}*/

real_point   translate(const real_vector& v) const;
/*{\Mop     returns \Mvar$+v$, i.e., |\Mvar| translated by vector
            $v$.\\
            \precond $v$.dim() = 2.}*/

real_point operator+(const real_vector& v) const { return translate(v); }
/*{\Mbinop  returns |\Mvar| translated by vector $v$.}*/

real_point operator-(const real_vector& v) const { return translate(-v); }
/*{\Mbinop  returns |\Mvar| translated by vector $-v$.}*/



real_point   rotate90(const real_point& q, int i=1) const;
/*{\Mopl    returns |\Mvar| rotated about $q$ by an angle of $i\times 90$
            degrees. If $i > 0$ the rotation is counter-clockwise otherwise
            it is clockwise. }*/

real_point   rotate90(int i=1) const;
/*{\Mop     returns |\Mvar|.rotate90($real\_point(0,0),i$). }*/


real_point reflect(const real_point& q, const real_point& r) const;
/*{\Mop     returns |\Mvar| reflected  across the straight line passing
            through $q$ and $r$.}*/

real_point reflect(const real_point& q) const;
/*{\Mop     returns |\Mvar| reflected across point $q$. }*/


bool operator==(const real_point& q) const;
bool operator!=(const real_point& q) const { return !operator==(q);}

real_vector operator-(const real_point& q)  const
{ return real_vector(xcoord()-q.xcoord(),ycoord()-q.ycoord()); }
/*{\Mbinop  returns the difference vector of the coordinates.}*/


static int  cmp_xy(const real_point&, const real_point&);
static int  cmp_yx(const real_point&, const real_point&);

static int  cmp_x(const real_point&, const real_point&);
static int  cmp_y(const real_point&, const real_point&);

friend __exportF istream& operator>>(istream& I, real_point& p) ;

friend inline unsigned long ID_Number(const real_point&);

};

inline ostream& operator<<(ostream& out, const real_point& p)
{ return out << "(" << p.xcoord() << "," << p.ycoord() << ")";}


COMPARE_DECL_PREFIX
inline int DEFAULT_COMPARE(const real_point& a, const real_point& b)
{
#if !defined(__BORLANDC__)
  if (identical(a,b)) return 0;
  int r = compare(a.xcoord(),b.xcoord());
  return (r!=0) ? r : compare(a.ycoord(),b.ycoord());
#else
  // Borland does not destroy the local vars correctly!
  return real_point::cmp_xy(a,b);
#endif
}



// geometric primitives

/*{\Mtext
{\bf Non-Member Functions}
\smallskip
}*/

inline int cmp_distances(const real_point& p1, const real_point& p2,
                         const real_point& p3, const real_point& p4)
{ return compare(p1.sqr_dist(p2),p3.sqr_dist(p4)); }
/*{\Mfuncl compares the distances |(p1,p2)| and |(p3,p4)|.
Returns $+1$ ($-1$) if distance |(p1,p2)| is larger (smaller) than
distance |(p3,p4)|, otherwise $0$.}*/

inline real_point center(const real_point& a, const real_point& b)
{ return real_point((a.xcoord()+b.xcoord())/2,(a.ycoord()+b.ycoord())/2); }
/*{\Mfuncl returns the center of $a$ and $b$, i.e. $a +\vec{ab}/2$. }*/

inline real_point midpoint(const real_point& a, const real_point& b) { return center(a,b); }
/*{\Mfuncl returns the center of $a$ and $b$. }*/



inline unsigned long ID_Number(const real_point& p) { return p.ptr()->id; }


inline int orientation(const real_point& a, const real_point& b, const real_point& c)
{ return a.orientation(b,c); }
/*{\Mfuncl computes the orientation of points $a$, $b$, and $c$ as
           the sign of the determinant\\

           \[ \left\Lvert \begin{array}{ccc} a_x & a_y & 1\\
                                        b_x & b_y & 1\\
                                        c_x & c_y & 1
                       \end{array} \right\Lvert \] \\

           i.e., it returns
           $+1$ if point $c$ lies left of the directed line through
           $a$ and $b$, $0$ if $a$,$b$, and $c$ are collinear, and
           $-1$ otherwise. }*/


inline int cmp_signed_dist(const real_point& a, const real_point& b, const real_point& c,
                                                           const real_point& d)
{ real d1 = (a.xcoord() - b.xcoord()) * (d.ycoord() - c.ycoord());
  real d2 = (a.ycoord() - b.ycoord()) * (d.xcoord() - c.xcoord());
  if (d1 == d2) return 0; else return (d1 > d2) ? +1 : -1;
}
/*{\Mfuncl compares (signed) distances of $c$ and $d$ to the straight line
           passing through $a$ and $b$ (directed from $a$ to $b$). Returns
           $+1$ ($-1$) if $c$ has larger (smaller) distance than $d$ and
           $0$ if distances are equal. }*/


inline real area(const real_point& a, const real_point& b, const real_point& c)
{ return a.area(b,c);}
/*{\Mfuncl computes the signed area of the triangle determined by $a$,$b$,$c$,
           positive if $orientation(a,b,c) > 0$ and negative otherwise. }*/


inline bool collinear(const real_point& a, const real_point& b, const real_point& c)
{ return (a.ycoord()-b.ycoord()) * (a.xcoord()-c.xcoord()) ==
         (a.xcoord()-b.xcoord()) * (a.ycoord()-c.ycoord()); }
/*{\Mfuncl returns |true| if points $a$, $b$, $c$ are collinear, i.e.,
           $orientation(a,b,c) = 0$, and |false| otherwise. }*/


inline bool right_turn(const real_point& a, const real_point& b, const real_point& c)
{ return (a.xcoord()-b.xcoord()) * (a.ycoord()-c.ycoord()) <
         (a.ycoord()-b.ycoord()) * (a.xcoord()-c.xcoord()); }
/*{\Mfuncl returns |true| if points $a$, $b$, $c$ form a righ turn, i.e.,
           $orientation(a,b,c) < 0$, and |false| otherwise. }*/


inline bool left_turn(const real_point& a, const real_point& b, const real_point& c)
{ return (a.xcoord()-b.xcoord()) * (a.ycoord()-c.ycoord()) >
         (a.ycoord()-b.ycoord()) * (a.xcoord()-c.xcoord()); }
/*{\Mfuncl returns |true| if points $a$, $b$, $c$ form a left turn, i.e.,
           $orientation(a,b,c) > 0$, and |false| otherwise. }*/


extern __exportF int side_of_halfspace(const real_point& a,
                                       const real_point& b,
                                       const real_point& c);
/*{\Mfuncl returns the sign of the scalar product $(b-a)\cdot(c-a)$. If
$b \not=a$ this amounts to: Let $h$ be the open halfspace orthogonal to the
vector $b - a$, containing $b$, and having $a$ in its boundary.
Returns $+1$ if $c$ is contained in
$h$, returns $0$ is $c$ lies on the the boundary of $h$, and returns $-1$ is
$c$ is contained in the interior of the complement of $h$.}*/

extern __exportF int side_of_circle(const real_point& a, const real_point& b,
                                    const real_point& c, const real_point& d);
/*{\Mfuncl returns $+1$ if point $d$ lies left of the directed circle through
           points $a$, $b$, and $c$, $0$ if $a$,$b$,$c$,and $d$ are
           cocircular, and $-1$ otherwise. }*/


inline bool inside_circle(const real_point& a, const real_point& b, const real_point& c,
                                                     const real_point& d)
{ return (orientation(a,b,c) * side_of_circle(a,b,c,d)) > 0; }
/*{\Mfuncl returns |true| if point $d$ lies in the interior of the circle
           through points $a$, $b$, and $c$, and |false| otherwise. }*/


inline bool outside_circle(const real_point& a, const real_point& b, const real_point& c,
                                                      const real_point& d)
{ return (orientation(a,b,c) * side_of_circle(a,b,c,d)) < 0; }
/*{\Mfuncl returns |true| if point $d$ lies outside of the circle
           through points $a$, $b$, and $c$, and |false| otherwise. }*/


inline bool on_circle(const real_point& a, const real_point& b, const real_point& c,
                                                       const real_point& d)
{ return side_of_circle(a,b,c,d) == 0; }
/*{\Mfuncl returns |true| if points $a$, $b$, $c$, and $d$ are cocircular. }*/



inline bool incircle(const real_point& a, const real_point& b, const real_point& c,
                                                     const real_point& d)
{ return inside_circle(a,b,c,d); }

inline bool outcircle(const real_point& a, const real_point& b, const real_point& c,
                                                      const real_point& d)
{ return outside_circle(a,b,c,d); }


inline bool cocircular(const real_point& a, const real_point& b, const real_point& c,
                                                       const real_point& d)
{ return side_of_circle(a,b,c,d) == 0; }
/*{\Mfuncl returns |true| if points $a$, $b$, $c$, and $d$ are cocircular. }*/

extern __exportF int compare_by_angle(const real_point& a, const real_point& b,
                                      const real_point& c, const real_point& d);
/*{\Mfuncl compares vectors |b-a| and |d-c| by angle (more efficient
           than calling |compare_by_angle(b-a,d-x)| on vectors).
}*/


extern __exportF bool affinely_independent(const array<real_point>& A);
/*{\Mfuncl decides whether the points in $A$ are affinely independent. }*/


extern __exportF bool contained_in_simplex(const array<real_point>& A,
                                           const real_point& p);
/*{\Mfuncl determines whether $p$ is contained in the simplex spanned
           by the points in |A|. |A| may consist of up to $3$
           points.\\
           \precond The points in |A| are affinely independent. }*/
  // precond : A contains points of different coordinates
  //           0 < A.size() < 4

extern __exportF bool contained_in_affine_hull(const array<real_point>& A,
                                               const real_point& p);
/*{\Mfuncl determines whether $p$ is contained in the affine hull
           of the points in $A$.}*/





inline const char* leda_tname(const real_point*) { return "real_point"; }


#if LEDA_ROOT_INCL_ID == 500280
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif


LEDA_END_NAMESPACE

#endif
