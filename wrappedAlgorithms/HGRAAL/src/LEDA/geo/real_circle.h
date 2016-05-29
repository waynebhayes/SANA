/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  real_circle.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:45:00 $


#ifndef LEDA_REAL_CIRCLE_H
#define LEDA_REAL_CIRCLE_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 442005
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/geo/circle.h>
#include <LEDA/geo/real_point.h>
#include <LEDA/geo/real_segment.h>
#include <LEDA/geo/real_line.h>
#include <LEDA/core/tuple.h>

LEDA_BEGIN_NAMESPACE

//------------------------------------------------------------------------------
// real circles
//------------------------------------------------------------------------------

class __exportC real_circle_rep : public handle_rep {

friend class __exportC real_circle;

  real_point a; 
  real_point b; 
  real_point c; 

  int orient;  // orientation(a,b,c)

  real D1;
  real D2;
  real D3;

  real_point*  cp; // pointer to center
  real* rp; // pointer to radius

  bool first_side_of;
  
public:

  real_circle_rep() {}
  real_circle_rep(const real_point&, const real_point&, const real_point&);
 ~real_circle_rep();

};


/*{\Manpage {real_circle} {} {Real Circles} {C}}*/ 


/*{\Mdefinition
An instance |\Mvar| of the data type |\Mname| is an oriented circle in the 
plane passing through three points $p_1$, $p_2$, $p_3$. The orientation of 
|\Mvar| is equal to the orientation of the three defining points, 
i.e. $orientation(p_1,p_2,p_3)$. 
If \Labs{\{p_1,p_2,p_3\}}$ = 1$ |\Mvar| is the empty circle with center $p_1$. 
If $p_1,p_2,p_3$ are collinear |\Mvar| is a straight line passing through
$p_1$, $p_2$ and $p_3$ in this order and the center of |\Mvar| is undefined. }*/

class __exportC rat_circle;

class __exportC real_circle   : public HANDLE_BASE(real_circle_rep) 
{

real_circle_rep* ptr() const { return (real_circle_rep*)PTR; }

public:

/*{\Mtypes 5}*/ 

typedef real       coord_type;
/*{\Mtypemember the coordinate type (|real|).}*/

typedef real_point point_type;
/*{\Mtypemember the point type (|real_point|).}*/

typedef circle     float_type;

/*{\Mcreation}*/ 

real_circle(const real_point& a, const real_point& b, const real_point& c);
/*{\Mcreate introduces a variable |\Mvar| of type |\Mname|. |\Mvar| is initialized to 
            the oriented circle through points $a$, $b$, and $c$. }*/

real_circle(const real_point& a, const real_point& b);
/*{\Mcreate introduces a variable |\Mvar| of type |\Mname|. |\Mvar| is initialized to 
            the counter-clockwise oriented circle with center $a$ passing
            through $b$.}*/

explicit

real_circle(const real_point& a);
/*{\Mcreate introduces a variable |\Mvar| of type |\Mname|. |\Mvar| is initialized to 
            the trivial circle with center $a$. }*/


real_circle();
/*{\Mcreate introduces a variable |\Mvar| of type |\Mname|. |\Mvar| is initialized to 
            the trivial circle with center $(0,0)$.}*/

real_circle(const real_point& c, real r);
/*{\Mcreate introduces a variable |\Mvar| of type |\Mname|. |\Mvar| is initialized to 
            the circle with center $c$ and radius $r$ with positive (i.e.
            counter-clockwise) orientation.}*/

real_circle(real x, real y, real r);
/*{\Mcreate introduces a variable |\Mvar| of type |\Mname|. |\Mvar| is initialized to 
            the circle with center $(x,y)$ and radius $r$ with positive 
			(i.e.~counter-clockwise) orientation.}*/


 real_circle(const circle& c, int prec = 0)
{ PTR = new real_circle_rep(real_point(c.point1(),prec), 
                            real_point(c.point2(),prec),
                            real_point(c.point3(),prec)); }
/*{\Mcreate introduces a variable |\Mvar| of type |\Mname| initialized
            to the circle $c$. 
            (The second argument is for compatibility with |rat_circle|.)}*/

 real_circle(const rat_circle& c);
/*{\Mcreate introduces a variable |\Mvar| of type |\Mname| initialized
            to the circle $c$.}*/

 real_circle(const real_circle& c) : HANDLE_BASE(real_circle_rep)(c) {}
~real_circle() {}

 real_circle& operator=(const real_circle& C) { HANDLE_BASE(real_circle_rep)::operator=(C); return *this; }


/*{\Moperations 2.2 4.9 }*/

real_point center()  const;
/*{\Mop  returns the center of |\Mvar|.\\
         \precond The orientation of |\Mvar| is not $0$.}*/

real radius() const;
/*{\Mop  returns the radius of |\Mvar|.\\
         \precond The orientation of |\Mvar| is not $0$.}*/

real sqr_radius() const;
/*{\Mop  returns the squared radius of |\Mvar|.\\
         \precond The orientation of |\Mvar| is not $0$.}*/

real_point point1() const { return ptr()->a; }
/*{\Mop  returns $p_1$. }*/

real_point point2() const { return ptr()->b; }
/*{\Mop  returns $p_2$. }*/

real_point point3() const { return ptr()->c; }
/*{\Mop  returns $p_3$. }*/

circle to_float() const { return to_circle(); }
circle to_circle() const 
{ return circle(point1().to_point(), point2().to_point(), point3().to_point()); }
/*{\Xop for compatibility with |rat_circle|. }*/

void    normalize() const {}
/*{\Xop  for compatibility with |rat_segment|. }*/

real_point point_on_circle(double alpha, double=0) const;
/*{\Xop  returns a point $p$ on |\Mvar| with angle of |alpha|. 
The second argument is for compatability with |rat_circle|. }*/


bool  is_degenerate() const { return ptr()->orient == 0; }
/*{\Mop returns true if the defining points are collinear.}*/

bool is_trivial() const { return ptr()->a == ptr()->b; }
/*{\Mop returns true if |\Mvar| has radius zero.}*/

int orientation()  const { return ptr()->orient; }
/*{\Mop  returns the orientation of |\Mvar|.}*/

int side_of(const real_point& p) const;
/*{\Mop   returns $-1$, $+1$, or $0$ if $p$ lies right of, left of, or on 
          |\Mvar| respectively. }*/

bool    inside(const real_point& p) const;
/*{\Mop   returns true iff $p$ lies inside of |\Mvar|.}*/

bool    outside(const real_point& p) const;
/*{\Mop   returns true iff $p$ lies outside of |\Mvar|.}*/


bool    contains(const real_point& p) const;
/*{\Mop   returns true iff $p$ lies on |\Mvar|.}*/


real_circle  translate(real dx, real dy) const;
/*{\Mop    returns |\Mvar| translated by vector $(dx,dy)$.}*/

real_circle  translate(const real_vector& v) const; 
/*{\Mop    returns |\Mvar| translated by vector $v$.}*/

real_circle  operator+(const real_vector& v) const { return translate(v); }
/*{\Mbinop   returns |\Mvar| translated by vector $v$.}*/

real_circle  operator-(const real_vector& v) const { return translate(-v); }
/*{\Mbinop   returns |\Mvar| translated by vector $-v$.}*/

real_circle rotate90(const real_point& q, int i=1) const;
/*{\Mopl    returns |\Mvar| rotated about $q$ by an angle of $i\times 90$ 
            degrees. If $i > 0$ the rotation is counter-clockwise otherwise
            it is clockwise. }*/

real_circle reflect(const real_point& p, const real_point& q) const;
/*{\Mop     returns |\Mvar| reflected  across the straight line passing
            through $p$ and $q$.}*/

real_circle reflect(const real_point& p) const;
/*{\Mop     returns |\Mvar| reflected  across point $p$.}*/

real_circle reverse() const { return real_circle(point3(),point2(),point1()); }
/*{\Mop     returns |\Mvar| reversed.}*/

list<real_point> intersection(const real_circle& D) const;
/*{\Mop    returns $C \cap D$ as a list of points.}*/

list<real_point> intersection(const real_line& l) const;
/*{\Mop  returns $C \cap l$ as a list of (zero, one, or two) points 
         sorted along $l$.}*/

list<real_point> intersection(const real_segment& s) const;
/*{\Mop  returns $C \cap s$ as a list of (zero, one, or two) points 
         sorted along $s$.}*/

real_segment left_tangent(const real_point& p) const;
/*{\Mop   returns the line segment starting in $p$ tangent 
	  to |\Mvar| and left of segment |[p,\Mvar.center()]|.}*/

real_segment right_tangent(const real_point& p) const;
/*{\Mop   returns the line segment starting in $p$ tangent 
	  to |\Mvar| and right of segment |[p,\Mvar.center()]|.}*/

real  distance(const real_point& p) const;
/*{\Mop   returns the distance between |\Mvar| and $p$. }*/

real  sqr_dist(const real_point& p) const;
/*{\Mop   returns the squared distance between |\Mvar| and $p$. }*/

real  distance(const real_line& l) const;
/*{\Mop    returns the distance between |\Mvar| and $l$. }*/

real  distance(const real_circle& D) const;
/*{\Mop    returns the distance between |\Mvar| and $D$. }*/

int intersection(const real_line& l, two_tuple<real_point,real_point>& inter) const;

bool operator==(const real_circle& D) const;
bool operator!=(const real_circle& D) const { return !operator==(D); };

friend __exportF istream& operator>>(istream& in, real_circle& c);  


};

__exportF bool radical_axis(const real_circle& C1, const real_circle& C2, real_line& rad_axis);
/*{\Mfuncl if the radical axis for |C1| and |C2| exists, it is assigned to
           |rad_axis| and true is returned; otherwise the result is false.}*/

inline ostream& operator<<(ostream& out, const real_circle& c) 
{ return out << c.point1() << " " << c.point2() << " " << c.point3();} 

inline const char* leda_tname(const real_circle*) { return "real_circle"; }

#if LEDA_ROOT_INCL_ID == 442005
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif


LEDA_END_NAMESPACE

#endif
