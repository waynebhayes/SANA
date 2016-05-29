/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  real_rectangle.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.3 $  $Date: 2005/04/14 10:45:01 $

#ifndef LEDA_REAL_RECTANGLE_H
#define LEDA_REAL_RECTANGLE_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 442004
#include <LEDA/internal/PREAMBLE.h>
#endif



#include <LEDA/core/list.h>

#include <LEDA/geo/rectangle.h>
#include <LEDA/numbers/real_vector.h>
#include <LEDA/geo/real_point.h>
#include <LEDA/geo/real_segment.h>
#include <LEDA/geo/real_line.h>
#include <LEDA/geo/real_circle.h>
#include <LEDA/geo/real_polygon.h>
#include <LEDA/geo/real_gen_polygon.h>

LEDA_BEGIN_NAMESPACE

//------------------------------------------------------------------------------
// iso-oriented rectangles
//
// 03/2002 : difference and intersection test for 2 rectangles added
// 08/2002 : region_of, center added
//
//------------------------------------------------------------------------------

class __exportC real_rectangle;

class __exportC real_rectangle_rep : public handle_rep {

  friend class __exportC real_rectangle;

  static leda_mutex mutex_id_counter;
  static unsigned long id_counter;

  real_point  p_ll;    // lower left point
  real_point  p_lr;    // lower right point
  real_point  p_ur;    // upper right point
  real_point  p_ul;    // upper left point

  unsigned long id;

 public:
   

   real_rectangle_rep();
   real_rectangle_rep(const real_point& p1, const real_point& p2);
   real_rectangle_rep(const real_point& p1, real w, real h);

   ~real_rectangle_rep() {}

   friend inline unsigned long ID_Number(const real_rectangle&);

};

/*{\Manpage {real_rectangle} {} {Iso-oriented Real Rectangles} {r}}*/

class __exportC rat_rectangle;

class __exportC real_rectangle  : public HANDLE_BASE(real_rectangle_rep) 
{

/*{\Mdefinition
An instance $r$ of the data type |\Mname| is an iso-oriented rectangle
in the two-dimensional plane. }*/

real_rectangle_rep* ptr() const { return (real_rectangle_rep*)PTR; }

real_point get_out_point(const real_ray& r) const;

public:

typedef real       coord_type;
typedef real_point point_type;
typedef rectangle  float_type;

/*{\Mcreation}*/

 real_rectangle(const real_point& p, const real_point& q);
 /*{\Mcreate 
 introduces a variable |\Mvar| of type |\Mname|. |\Mvar| is initialized to the 
 |\Mname| with diagonal corners |p| and |q| }*/

 real_rectangle(const real_point& p, real w, real h);
 /*{\Mcreate
 introduces a variable |\Mvar| of type |\Mname|. |\Mvar| is initialized to the 
 |\Mname| with lower left corner |p|, width |w| and height |h|. }*/

 real_rectangle(real x1,real y1,real x2,real y2);
 /*{\Mcreate
 introduces a variable |\Mvar| of type |\Mname|. |\Mvar| is initialized to the 
 |\Mname| with diagonal corners |(x1,y1)| and |(x2,y2)|. }*/ 

 real_rectangle(const rectangle& r1, int prec = 0)
 { PTR = new real_rectangle_rep(real_point(r1.upper_left(), prec),
	                            real_point(r1.lower_right(), prec)); }
/*{\Mcreate introduces a variable |\Mvar| of type |\Mname| initialized
            to the rectangle $r_1$. 
            (The second argument is for compatibility with |rat_rectangle|.)}*/

 real_rectangle(const rat_rectangle& r1);
/*{\Mcreate introduces a variable |\Mvar| of type |\Mname| initialized
            to the rectangle $r_1$.}*/

 real_rectangle();
 
 real_rectangle(const real_rectangle& r) : HANDLE_BASE(real_rectangle_rep)(r) {};

 real_rectangle& operator=(const real_rectangle& r) { HANDLE_BASE(real_rectangle_rep)::operator=(r); return *this; }
 
 ~real_rectangle() {}
 
/*{\Moperations 2 5.0 }*/

rectangle to_float() const { return to_rectangle(); }
rectangle to_rectangle() const { return rectangle(upper_left().to_point(),lower_right().to_point()); }
/*{\Xop for compatibility with |rat_rectangle|. }*/

void    normalize() const {}
/*{\Xop  for compatibility with |rat_rectangle|. }*/


 real_point upper_left() const;
/*{\Mop    returns the upper left corner.}*/

 real_point upper_right() const;
/*{\Mop    returns the upper right corner.}*/

 real_point lower_left() const;
/*{\Mop    returns the lower left corner.}*/

 real_point lower_right() const;
/*{\Mop    returns the lower right corner.}*/

 real_point center() const;
/*{\Mop    returns the center of |\Mvar|.}*/

 list<real_point> vertices() const;
/*{\Mop    returns the vertices of |\Mvar| in counterclockwise order starting from the lower left point.}*/

 real xmin() const;
/*{\Mop    returns the minimal x-coordinate of |\Mvar|.}*/

 real xmax() const;
/*{\Mop    returns the maximal x-coordinate of |\Mvar|.}*/

 real ymin() const;
/*{\Mop    returns the minimal y-coordinate of |\Mvar|.}*/

 real ymax() const;
/*{\Mop    returns the maximal y-coordinate of |\Mvar|.}*/

 real width() const;
/*{\Mop    returns the width of |\Mvar|.}*/

 real height() const;
/*{\Mop    returns the height of |\Mvar|.}*/

 bool is_degenerate() const;
/*{\Mop    returns true, if |\Mvar| degenerates to a segment or point (the 4 corners are collinear),
           false otherwise.}*/

 bool is_point() const;
/*{\Mop    returns true, if |\Mvar| degenerates to a point.}*/

 bool is_segment() const;
/*{\Mop    returns true, if |\Mvar| degenerates to a segment.}*/

 int  cs_code(const real_point& p) const;
/*{\Mop    returns the code for Cohen-Sutherland algorithm.}*/

 bool inside(const real_point& p) const;
/*{\Mop    returns true, if p is inside of |\Mvar|, false otherwise.}*/

 bool outside(const real_point& p) const;
/*{\Mop    returns true, if p is outside of |\Mvar|, false otherwise.}*/

 bool inside_or_contains(const real_point& p) const;
/*{\Mop    returns true, if p is inside of |\Mvar| or on the border, false otherwise.}*/

 bool contains(const real_point& p) const; 
/*{\Mop    returns true, if p is on the border of |\Mvar|, false otherwise.}*/

 region_kind region_of(const real_point& p) const;
/*{\Mop     returns BOUNDED\_REGION if $p$ lies in the 
            bounded region of |\Mvar|, returns ON\_REGION if $p$ lies on 
            |\Mvar|, and returns UNBOUNDED\_REGION if $p$ lies in the 
            unbounded region. }*/

 real_rectangle include(const real_point& p) const;
/*{\Mop    returns a new rectangle that includes the points of |\Mvar| and p.}*/

 real_rectangle include(const real_rectangle& r2) const;
/*{\Mop    returns a new rectangle that includes the points of |\Mvar| and r2.}*/

 real_rectangle translate(real dx,real dy) const;
/*{\Mop    returns a new rectangle that is the translation of |\Mvar| by |(dx,dy)|.}*/

 real_rectangle translate(const real_vector& v) const;
/*{\Mop    returns a new rectangle that is the translation of |\Mvar| by |v|.}*/

 real_rectangle operator+(const real_vector& v) const { return translate(v); }
/*{\Mbinop returns |\Mvar| translated by  |v|.}*/

 real_rectangle operator-(const real_vector& v) const { return translate(-v); }
/*{\Mbinop returns |\Mvar| translated by |-v|.}*/

 real_point operator[](int i) const;
/*{\Marrop returns the |i-th| vertex of |\Mvar|. Precondition: |(0<i<5)|.}*/

 real_rectangle rotate90(const real_point& p, int i=1) const;
/*{\Mopl    returns |\Mvar| rotated about $p$ by an angle of $i\times 90$ 
            degrees. If $i > 0$ the rotation is counter-clockwise otherwise
            it is clockwise. }*/

 real_rectangle rotate90(int i=1) const;
/*{\Mop    returns |\Mvar| rotated by an angle of $i\times 90$ degrees 
           about the origin.}*/

 real_rectangle reflect(const real_point& p) const;
/*{\Mop    returns |\Mvar| reflected across |p| .}*/

 list<real_point> intersection(const real_segment& s) const;
/*{\Mop    returns $\Mvar \cap s$ .}*/

 bool      intersect(real_point& ps,const real_segment& s,int c1,int c2) const;

 bool      clip(const real_segment& t,real_segment& inter) const;
/*{\Mop    clips |t| on |\Mvar| and returns the result in |inter|.}*/

 bool      clip(const real_segment& t,real_segment& inter,int c1,int c2) const;

 bool      clip(const real_line& l,real_segment& inter) const;
/*{\Mop    clips |l| on |\Mvar| and returns the result in |inter|.}*/

 bool      clip(const real_ray& ry,real_segment& inter) const;
/*{\Mop    clips |ry| on |\Mvar| and returns the result in |inter|.}*/

 bool      difference(const real_rectangle& q, list<real_rectangle>& L) const;
/*{\Mop    returns |true| iff the difference of \Mvar and |q| is not empty, 
           and |false| otherwise. The difference |L| is returned as a 
           partition into rectangles.}*/

 list<real_point> intersection(const real_line& l) const;
/*{\Mop    returns $\Mvar \cap l$.}*/

 list<real_rectangle> intersection(const real_rectangle& s) const;
/*{\Mop    returns $\Mvar \cap s$.}*/

 bool      do_intersect(const real_rectangle& b) const;
/*{\Mop    returns |true| iff \Mvar and $b$ intersect, false otherwise.}*/

 real area() const;
/*{\Mop    returns the area of |\Mvar|.}*/

 bool operator==(const real_rectangle& s) const 
 { if (s.lower_left() == lower_left() && s.upper_right() == upper_right()) return true; else return false; }

 bool operator!=(const real_rectangle& s) const  { return !operator==(s); }

 friend inline ostream& operator<<(ostream& out, const real_rectangle& r);
 friend inline istream& operator>>(istream& in, real_rectangle& r);  

 friend inline unsigned long ID_Number(const real_rectangle&);
};

inline ostream& operator<<(ostream& out, const real_rectangle& r) 
{ return out << r.lower_left() << r.upper_right(); }

inline istream& operator>>(istream& in, real_rectangle& r)  
{ real_point p1,p2; 
  in >> p1;
  in >> p2; 
  r = real_rectangle(p1,p2); 
  return in; 
}

inline const char* leda_tname(const real_rectangle*) { return "real_rectangle"; }

inline unsigned long  ID_Number(const real_rectangle& r) { return r.ptr()->id; }

// BoundingBox functions

extern __exportF real_rectangle BoundingBox(const real_point& p);
extern __exportF real_rectangle BoundingBox(const real_segment& s);
extern __exportF real_rectangle BoundingBox(const real_ray& r);
extern __exportF real_rectangle BoundingBox(const real_line& l);
extern __exportF real_rectangle BoundingBox(const real_circle& c);
extern __exportF real_rectangle BoundingBox(const real_polygon& P);
extern __exportF real_rectangle BoundingBox(const real_gen_polygon& P);

#if LEDA_ROOT_INCL_ID == 442004
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif


LEDA_END_NAMESPACE

#endif
