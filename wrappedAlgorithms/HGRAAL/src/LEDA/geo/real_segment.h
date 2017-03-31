/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  real_segment.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.3 $  $Date: 2005/04/14 10:45:01 $

#ifndef LEDA_REAL_SEGMENT_H
#define LEDA_REAL_SEGMENT_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 442001
#include <LEDA/internal/PREAMBLE.h>
#endif


#include <LEDA/geo/real_point.h>
#include <LEDA/geo/segment.h>

LEDA_BEGIN_NAMESPACE

//------------------------------------------------------------------------------
// real segments
//------------------------------------------------------------------------------

class __exportC real_segment;

class __exportC real_segment_rep : public handle_rep {

friend class __exportC real_segment;
friend class __exportC real_line;
friend class __exportC real_circle;

static leda_mutex mutex_id_counter;
static unsigned long id_counter;

   unsigned long id;

   real_point start;
   real_point end;

   real dx;
   real dy;

public:
   
   real_segment_rep(const real_point&, const real_point&);
  ~real_segment_rep() {}

   friend inline unsigned long ID_Number(const real_segment&);

};

/*{\Manpage {real_segment} {} {Real Segments} {s}}*/

class __exportC rat_segment;

class __exportC real_segment  : public HANDLE_BASE(real_segment_rep)
{
/*{\Mdefinition
    An instance $s$ of the data type |\Mname| is a directed straight line
    segment in the two-dimensional plane, i.e., a straight line segment $[p,q]$
    connecting two points $p,q \in \real ^2$. $p$ is called the {\em source} or 
    start point and $q$ is called the {\em target} or end point of $s$. The 
    length of $s$ is the Euclidean distance between $p$ and $q$. If $p = q$, 
    $s$ is called empty. We use |line(s)| to denote a straight line
    containing $s$.}*/

friend class __exportC real_line;
friend class __exportC real_circle;

real_segment_rep* ptr() const { return (real_segment_rep*)PTR; }

public:

/*{\Mtypes 5}*/ 

typedef real       coord_type;
/*{\Mtypemember the coordinate type (|real|).}*/

typedef real_point point_type;
/*{\Mtypemember the point type (|real_point|).}*/

typedef segment    float_type;



/*{\Mcreation}*/

real_segment(const real_point& p, const real_point& q); 
/*{\Mcreate introduces a variable |\Mvar| of type |\Mname|. |\Mvar| is 
            initialized to the segment $[p,q]$. }*/

real_segment(const real_point& p, const real_vector& v); 
/*{\Mcreate introduces a variable |\Mvar| of type |\Mname|. |\Mvar| is 
            initialized to the segment $[p,p+v]$.\\
	    \precond $v.dim() = 2$. }*/

real_segment(real x1, real y1, real x2, real y2) ;
/*{\Mcreate introduces a variable |\Mvar| of type |\Mname|. |\Mvar| is 
            initialized to the segment $[(x_1,y_1),(x_2,y_2)]$.}*/ 

real_segment();                 
/*{\Mcreate introduces a variable |\Mvar| of type |\Mname|. |\Mvar| is 
            initialized to the empty segment.}*/

real_segment(const segment& s1, int prec = 0)
{ PTR = new real_segment_rep(real_point(s1.source(),prec), 
                             real_point(s1.target(),prec)); }
/*{\Mcreate introduces a variable |\Mvar| of type |\Mname| initialized
            to the segment $s_1$. 
            (The second argument is for compatibility with |rat_segment|.)}*/

real_segment(const rat_segment& s1);
/*{\Mcreate introduces a variable |\Mvar| of type |\Mname| initialized
            to the segment $s_1$.}*/

 real_segment(const real_segment& s) : HANDLE_BASE(real_segment_rep)(s) {}     
~real_segment() {}
 real_segment& operator=(const real_segment& s) 
 { HANDLE_BASE(real_segment_rep)::operator=(s); return *this;}


/*{\Moperations 2 3.5}*/

segment  to_segment() const { return segment(start().to_point(),end().to_point()); }
segment  to_float() const { return to_segment(); }
/*{\Xop  for compatibility with rat_segment. }*/

void    normalize() const {}
/*{\Xop  for compatibility with |rat_segment|. }*/


/*{\Moptions nextwarning=no}*/
real_point start()  const      { return ptr()->start; }
real_point source() const      { return ptr()->start; }
/*{\Mop       returns the source point of segment |\Mvar|.}*/

/*{\Moptions nextwarning=no}*/
real_point end()    const      { return ptr()->end; }
real_point target() const      { return ptr()->end; }
/*{\Mop       returns the target point of segment |\Mvar|.}*/

real xcoord1() const    { return ptr()->start.ptr()->x; }
/*{\Mop       returns the x-coordinate of |\Mvar|.source().}*/

real xcoord2() const    { return ptr()->end.ptr()->x;   }
/*{\Mop       returns the x-coordinate of |\Mvar|.target().}*/

real ycoord1() const    { return ptr()->start.ptr()->y; }
/*{\Mop       returns the y-coordinate of |\Mvar|.source().}*/

real ycoord2() const    { return ptr()->end.ptr()->y;   }
/*{\Mop       returns the y-coordinate of |\Mvar|.target().}*/


operator real_vector()  
{ return real_vector(xcoord2()-xcoord1(), ycoord2()-ycoord1()); }


real dx() const    { return ptr()->dx;   }
/*{\Mop       returns the $xcoord2 - xcoord1$.}*/

real dy() const    { return ptr()->dy;   }
/*{\Mop       returns the $ycoord2 - ycoord1$.}*/


real slope() const;
/*{\Mop       returns the slope of $s$.\\
	      \precond  |\Mvar|  is not vertical.}*/


real sqr_length() const;
/*{\Mop       returns the square of the length of |\Mvar|.}*/

real length() const;
/*{\Mop       returns the length of |\Mvar|.}*/

real_vector  to_vector() const { return target() - source(); }
/*{\Mop returns the vector |\Mvar.target() - \Mvar.source()|. }*/

bool is_trivial() const { return is_vertical() && is_horizontal(); }
/*{\Mopl    returns true if |\Mvar| is trivial. }*/


bool is_vertical()   const { return xcoord1() == xcoord2(); }
/*{\Mop       returns true iff |\Mvar| is vertical.}*/

bool is_horizontal() const { return ycoord1() == ycoord2(); }
/*{\Mop       returns true iff |\Mvar| is horizontal.}*/

int orientation(const real_point& p) const
{ return (dy()*(xcoord1()-p.xcoord()) - dx()*(ycoord1()-p.ycoord())).sign(); }
/*{\Mop   computes orientation($\Mvar.source()$, $\Mvar.target()$, $p$) (see below).}*/

real  x_proj(real y) const;
/*{\Mop    returns $p$.xcoord(), where $p \in line(|\Mvar|)$ with 
           $p$.ycoord() = $y$.\\
	   \precond |\Mvar| is not horizontal.}*/

real  y_proj(real x) const;
/*{\Mop    returns $p$.ycoord(), where $p \in line(|\Mvar|)$ with 
           $p$.xcoord() = $x$.\\
           \precond |\Mvar| is not vertical.}*/

real  y_abs() const;
/*{\Mop    returns the y-abscissa of $line(\Mvar)$, i.e., |\Mvar|.y\_proj(0).\\
	   \precond  |\Mvar|  is not vertical.}*/


bool   contains(const real_point& p) const;
/*{\Mopl    decides whether |\Mvar| contains $p$. }*/


bool intersection(const real_segment& t) const;
/*{\Mopl    decides whether |\Mvar| and $t$ intersect in one point. }*/


bool intersection(const real_segment& t, real_point& p) const;
/*{\Mopl    if |\Mvar| and $t$ intersect in a single point this point
	    is assigned to $p$ and the result is true, otherwise the 
             result is false.}*/

bool intersection_of_lines(const real_segment& t, real_point& p) const;
/*{\Mopl    if $line(\Mvar)$ and $line(t)$ intersect in a single point
            this point is assigned to $p$ and the result is true, otherwise
            the result is false.}*/


real_segment translate(real dx, real dy) const;
/*{\Mopl    returns |\Mvar| translated by vector $(dx,dy)$.}*/

real_segment translate(const real_vector& v) const;
/*{\Mop     returns $s+v$, i.e.,  |\Mvar| translated by vector $v$.\\
	    \precond $v$.dim() = 2.}*/ 

real_segment operator+(const real_vector& v) const { return translate(v); }
/*{\Mbinop  returns |\Mvar| translated by vector $v$.}*/

real_segment operator-(const real_vector& v) const { return translate(-v); }
/*{\Mbinop  returns |\Mvar| translated by vector $-v$.}*/


real_segment perpendicular(const real_point& p) const;
/*{\Mop    returns the segment perpendicular to |\Mvar| with source $p$
           and target on $line(\Mvar)$.}*/

real  distance(const real_point& p) const;
/*{\Mopl   returns the Euclidean distance between $p$ and $|\Mvar|$.}*/

real  sqr_dist(const real_point& p) const;
/*{\Mopl   returns the squared Euclidean distance between $p$ and $|\Mvar|$.}*/

real  distance() const { return distance(real_point(0,0)); }
/*{\Mopl   returns the Euclidean distance between $(0,0)$ 
           and $|\Mvar|$.}*/

real_segment rotate90(const real_point& q, int i=1) const;
/*{\Mopl    returns |\Mvar| rotated about $q$ by an angle of $i\times 90$ 
            degrees. If $i > 0$ the rotation is counter-clockwise otherwise
            it is clockwise. }*/

real_segment rotate90(int i=1) const;
/*{\Mop     returns $s$.rotate90($s$.source(),i).}*/


real_segment reflect(const real_point& p, const real_point& q) const;
/*{\Mop     returns |\Mvar| reflected  across the straight line passing
            through $p$ and $q$.}*/

real_segment reflect(const real_point& p) const;
/*{\Mop     returns |\Mvar| reflected  across point $p$.}*/


real_segment reverse() const { return real_segment(target(),source()); }
/*{\Mop     returns |\Mvar| reversed.}*/


bool operator==(const real_segment& t) const
{ return (ptr()->start == t.ptr()->start && ptr()->end == t.ptr()->end); }

bool operator!=(const real_segment& t) const { return !operator==(t);}


friend __exportF istream& operator>>(istream& I, real_segment& s);

friend inline unsigned long ID_Number(const real_segment&);

};



inline ostream& operator<<(ostream& out, const real_segment& s) 
{ return out << "[" << s.start() << "===" << s.end() << "]"; } 



/*{\Mtext
{\bf Non-Member Functions}
\smallskip
}*/

inline unsigned long ID_Number(const real_segment& s) { return s.ptr()->id; }


inline int orientation(const real_segment& s, const real_point& p)
{ return s.orientation(p); }
/*{\Mfuncl   computes orientation($s.source()$, $s.target()$, $p$).}*/


inline int cmp_slopes(const real_segment& s1, const real_segment& s2)
/*{\Mfuncl   returns compare(slope($s_1$), slope($s_2$)).}*/
{ if (!s1.is_vertical()) {
	if (!s2.is_vertical()) return compare(s1.slope(), s2.slope());
	else                   return -1;
  }
  else {
	if (!s2.is_vertical()) return +1;
	else                   return 0;
  }
}

extern __exportF int cmp_segments_at_xcoord(const real_segment& s1,
                                            const real_segment& s2,
                                            const real_point& p);
/*{\Mfuncl      compares points $l_1 \cap v$ and $l_2 \cap v$ where
                $l_i$ is the line underlying segment $s_i$ and $v$ is
                the vertical straight line passing through point $p$. }*/


inline bool parallel(const real_segment& s1, const real_segment& s2)
/*{\Mfuncl   returns true if $s1$ and $s2$ are parallel and false otherwise.}*/
{ return cmp_slopes(s1,s2) == 0; }


inline const char* leda_tname(const real_segment*) { return "real_segment"; }

#if LEDA_ROOT_INCL_ID == 442001
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif


LEDA_END_NAMESPACE

#endif

