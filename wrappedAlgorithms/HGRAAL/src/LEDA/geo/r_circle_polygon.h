/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  r_circle_polygon.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.3 $  $Date: 2005/04/14 10:44:56 $


#ifndef LEDA_R_CIRCLE_POLYGON_H
#define LEDA_R_CIRCLE_POLYGON_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500268
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/geo/r_circle_segment.h>
#include <LEDA/geo/geo_global_enums.h>
#include <LEDA/core/tuple.h>

LEDA_BEGIN_NAMESPACE

//------------------------------------------------------------------------------
// r_circle_polygon:   polygon with r_circle_segment edges
//------------------------------------------------------------------------------

/// r_circle_polygon /////////////////////////////////////////////////////////////////////////

class r_circle_polygon_rep : public handle_rep {
	struct BBox {
		real xmin, ymin, xmax, ymax;
	};

public:
	r_circle_polygon_rep()
	 : Orientation(0), pBoundingBox(0) {}

	r_circle_polygon_rep(const list<r_circle_segment>& segs, int orient)
	 : Segments(segs), Orientation(orient), pBoundingBox(0) {}

	virtual ~r_circle_polygon_rep() 
	{ delete pBoundingBox; pBoundingBox = 0; }

private:
	list<r_circle_segment> Segments;
	int                    Orientation;
	BBox*                  pBoundingBox;

	friend class __exportC r_circle_polygon;
};

class __exportC rat_polygon;
class __exportC polygon;

/*{\Manpage {r_circle_polygon} {} {Polygons with circular edges} {P}}*/

class __exportC r_circle_polygon : public HANDLE_BASE(r_circle_polygon_rep) {
/*{\Mdefinition
An instance |\Mvar| of the data type |\Mtype| is a cyclic list of 
|r_circle_segments|, i.e.~straight line or circular segments.
A polygon is called \emph{simple} if all nodes of the graph induced by 
its segments have degree two and it is called \emph{weakly simple}, if 
its segments are disjoint except for common endpoints and if the chain 
does not cross itself. 
See the LEDA book for details.

A weakly simple polygon splits the plane into an unbounded region and
one or more bounded regions. For a simple polygon there is just one
bounded region. When a weakly simple
polygon |\Mvar| is traversed either the bounded region is consistently to
the left of |\Mvar| or the unbounded region is consistently to the left of 
|\Mvar|.
We say that |\Mvar| is positively oriented in the former case and negatively
oriented in the latter case. We use |\Mvar| to also denote the region to the 
left of |\Mvar| and call this region the positive side of |\Mvar|.

The number of segments is called the \emph{size} of |\Mvar|. 
A polygon of size zero is \emph{trivial}; it either describes the empty set 
or the full two-dimensional plane.
}*/

public:
	typedef r_circle_polygon_rep rep;
	typedef HANDLE_BASE(rep)     base;

private:
	rep* ptr() const { return (rep*) PTR; }

public:
/*{\Mtypes}*/

	enum KIND { EMPTY, FULL, NON_TRIVIAL };
	/*{\Menum describes the kind of the polygon: the empty set, the full plane 
	          or a non-trivial polygon.}*/

	enum CHECK_TYPE { NO_CHECK = 0, SIMPLE = 1, WEAKLY_SIMPLE = 2, NOT_WEAKLY_SIMPLE = 3 };
	/*{\Menum used to specify which checks should be applied and also describes
	          the outcome of a simplicity check.}*/

	enum RESPECT_TYPE { DISREGARD_ORIENTATION = 0, RESPECT_ORIENTATION = 1 };
	/*{\Menum used in contructors to specify whether to force a positive
	          orientation for the constructed object (|DISREGARD_ORIENTATION|)
			  or to keep the orientation of the input (|RESPECT_ORIENTATION|).}*/

public:
/*{\Mcreation}*/

	r_circle_polygon() { PTR = new rep; }
	/*{\Mcreate creates an empty polygon |\Mvar|.}*/

	r_circle_polygon(KIND k);
	/*{\Mcreate creates a polygon |\Mvar| of kind |k|, where |k| is either 
	            |EMPTY| or |FULL|.}*/

	r_circle_polygon(const list<r_circle_segment>& chain, 
					 CHECK_TYPE check = WEAKLY_SIMPLE, RESPECT_TYPE respect_orient = RESPECT_ORIENTATION)
	{ init(chain, check, respect_orient); }
	/*{\Mcreate creates a polygon |\Mvar| from a closed chain of segments.}*/

	r_circle_polygon(const list<rat_point>& L,
					 CHECK_TYPE check = WEAKLY_SIMPLE, RESPECT_TYPE respect_orient = RESPECT_ORIENTATION)
	{ init(L, check, respect_orient); }
	/*{\Mcreate creates a polygon |\Mvar| with straight line edges from a 
	            list |L| of vertices.}*/

	r_circle_polygon(const rat_polygon& Q,
					 CHECK_TYPE check = NO_CHECK, RESPECT_TYPE respect_orient = RESPECT_ORIENTATION);
	/*{\Mcreate converts a |rat_polygon| $Q$ to an |\Mtype| |\Mvar|.}*/

	r_circle_polygon(const polygon& Q, 
					 CHECK_TYPE check = NO_CHECK, RESPECT_TYPE respect_orient = RESPECT_ORIENTATION, 
					 int prec = rat_point::default_precision);
	/*{\Mcreate converts the (floating point) |polygon| $Q$ to an |\Mtype|.
	            |\Mvar| is initialized to a rational approximation of $Q$ of 
				coordinates with denominator at most |prec|. 
				If |prec| is zero, the implementation chooses |prec| large 
				enough such that there is no loss of precision in the 
				conversion.}*/

	r_circle_polygon(const rat_circle& circ, RESPECT_TYPE respect_orient = RESPECT_ORIENTATION);
	/*{\Mcreate creates a polygon |\Mvar| whose boundary is the 
	            circle |circ|.}*/

	r_circle_polygon(const r_circle_polygon& P) : base(P) {}

	const r_circle_polygon& operator=(const r_circle_polygon& p)
	{ base::operator=(p); return *this; }

/*{\Moperations}*/

	KIND kind() const 
	{ return is_trivial() ? (ptr()->Orientation == 0 ? EMPTY : FULL) : NON_TRIVIAL; }
	/*{\Mop     returns the kind of |\Mvar|.}*/

	bool is_trivial() const { return ptr()->Segments.empty(); }
	/*{\Mop     returns true iff |\Mvar| is trivial.}*/

	/*{\Moptions nextwarning=no}*/
	bool is_empty() const { return ptr()->Orientation == 0; }
	bool empty() const { return is_empty(); } // for compatibility with POLYGON
	/*{\Mop     returns true iff |\Mvar| is empty.}*/

	bool is_full() const { return kind() == FULL; }
	/*{\Mop     returns true iff |\Mvar| is the full plane.}*/

	void normalize();
	/*{\Mop     simplifies the representation by calling
	            |s.normalize()| for every segment $s$ of |\Mvar|.}*/

	bool is_simple() const
	{ return check_chain_for_simplicity(segments()) == SIMPLE; }
	/*{\Mop   tests whether |\Mvar| is simple.}*/

	bool is_weakly_simple() const
	{ return check_chain_for_simplicity(segments()) != NOT_WEAKLY_SIMPLE; }
	/*{\Mop   tests whether |\Mvar| is weakly simple.}*/

	bool is_weakly_simple(list<r_circle_point>& crossings) const
	{ return check_chain_for_simplicity(segments(), &crossings) != NOT_WEAKLY_SIMPLE; }
	/*{\Mop   as above, returns all proper points of intersection in 
	          |crossings|. }*/

	CHECK_TYPE check_simplicity() const
	{ return check_chain_for_simplicity(segments()); }
	/*{\Mop   checks |\Mvar| for simplicity. The result can 
	          be |SIMPLE|, |WEAKLY_SIMPLE| or |NOT_WEAKLY_SIMPLE|. }*/

	bool is_convex() const;
	/*{\Mop     returns true iff |\Mvar| is convex.}*/

	int size() const { return segments().size(); }
	/*{\Mop     returns the size of |\Mvar|.}*/

	/*{\Moptions nextwarning=no}*/
	const list<r_circle_segment>& segments() const { return ptr()->Segments; }
	const list<r_circle_segment>& edges() const { return segments(); }
	/*{\Mop     returns a chain of segments that bound |\Mvar|. 
	            The orientation of the chain corresponds to the orientation 
				of |\Mvar|.}*/

	list<r_circle_point> vertices() const;
	/*{\Mop     returns the vertices of |\Mvar|.}*/

	list<r_circle_point> intersection(const r_circle_segment& s) const;
	/*{\Mopl    returns the list of all proper intersections between $s$ and the
	            boundary of |\Mvar|.}*/

	list<r_circle_point> intersection(const rat_line& l) const;
	/*{\Mopl    returns the list of all proper intersections between $l$ and the
	            boundary of |\Mvar|.}*/

	/*{\Moptions nextwarning=no}*/
	r_circle_polygon clip_against_halfplane(const rat_line& l) const;
	r_circle_polygon intersection_halfplane(const rat_line& l) const
	{ return clip_against_halfplane(l); } // for compatibility
	/*{\Mopl    clips |\Mvar| against the halfplane on the positive side of $l$.
	            Observe that the result is only guaranteed to be weakly simple if
	            |\Mvar| is convex.}*/

	r_circle_polygon translate(rational dx, rational dy) const;
	/*{\Mopl    returns |\Mvar| translated by vector $(dx,dy)$.}*/

	r_circle_polygon translate(const rat_vector& v) const { return translate(v[0], v[1]); }
	/*{\Mop     returns |\Mvar| translated by vector $v$.}*/

	r_circle_polygon operator+(const rat_vector& v) const { return translate(v); }
	/*{\Mbinop  returns |\Mvar| translated by vector $v$.}*/

	r_circle_polygon operator-(const rat_vector& v) const { return translate(-v); }
	/*{\Mbinop  returns |\Mvar| translated by vector $-v$.}*/

	r_circle_polygon rotate90(const rat_point& q, int i=1) const;
	/*{\Mopl    returns |\Mvar| rotated about $q$ by an angle of $i\times 90$ 
				degrees. If $i > 0$ the rotation is counter-clockwise otherwise
				it is clockwise. }*/

	r_circle_polygon reflect(const rat_point& p, const rat_point& q) const;
	/*{\Mop     returns |\Mvar| reflected  across the straight line passing
				through $p$ and $q$.}*/

	r_circle_polygon reflect(const rat_point& p) const;
	/*{\Mop     returns |\Mvar| reflected  across point $p$.}*/


	real sqr_dist(const real_point& p) const;
	/*{\Mop computes the squared Euclidean distance between the boundary of 
	        |\Mvar| and $p$. (If |\Mvar| is zero, the result is zero.)}*/

	real dist(const real_point& p) const { return sqrt(sqr_dist(p)); }
	/*{\Mop computes the Euclidean distance between the boundary of 
	        |\Mvar| and $p$. (If |\Mvar| is zero, the result is zero.)}*/


	r_circle_polygon complement() const;
	/*{\Mopl    returns the complement of |\Mvar|.}*/

	r_circle_polygon eliminate_cocircular_vertices() const;
	/*{\Mopl    returns a copy of |\Mvar| without cocircular vertices.}*/

	bool is_rat_polygon() const;
	/*{\Mopl    returns whether |\Mvar| can be converted to a |rat_polygon|.}*/

	rat_polygon to_rat_polygon() const;
	/*{\Mopl    converts |\Mvar| to a |rat_polygon|.\\ 
	            \precond |is_rat_polygon| is true.}*/

	polygon to_float() const;
	/*{\Mopl    computes a floating point approximation of |\Mvar| with 
	            straight line segments.\\ 
	            \precond |is_rat_polygon| is true.}*/

	bool is_rat_circle() const;
	/*{\Mopl    returns whether |\Mvar| can be converted to a |rat_circle|.}*/

	rat_circle to_rat_circle() const;
	/*{\Mopl    converts |\Mvar| to a |rat_circle|.\\ 
	            \precond |is_rat_circle| is true.}*/

	void bounding_box(real& xmin, real& ymin, real& xmax, real& ymax) const;
	/*{\Mop computes a tight bounding box for |\Mvar|.}*/

/*{\Mtext 
\medskip
{\bf All functions below assume that |\Mvar| is weakly simple.}
\medskip
}*/

	int orientation() const { return ptr()->Orientation; }
	/*{\Mop     returns the orientation of |\Mvar|. }*/

	int         side_of(const r_circle_point& p) const;
	/*{\Mop     returns $+1$ if $p$ lies to the left of |\Mvar|, $0$ if 
				$p$ lies on |\Mvar|, and $-1$ if $p$ lies to the 
				right of |\Mvar|.}*/

	region_kind region_of(const r_circle_point& p) const;
	/*{\Mop     returns BOUNDED\_REGION if $p$ lies in the 
				bounded region of |\Mvar|, returns ON\_REGION if $p$ lies on 
				|\Mvar|, and returns UNBOUNDED\_REGION if $p$ lies in the 
				unbounded region. }*/

	bool inside(const r_circle_point& p) const  
	{ return side_of(p) == +1; }
	/*{\Mop     returns true if $p$ lies to the left of |\Mvar|, i.e.,
				|side_of(p) == +1|. }*/

	bool on_boundary(const r_circle_point& p) const  
	{ return side_of(p) == 0; }
	/*{\Mop     returns true if $p$ lies on |\Mvar|, i.e., 
				|side_of(p) == 0|. }*/

	bool outside(const r_circle_point& p) const 
	{ return side_of(p) == -1; }
	/*{\Mop     returns true if $p$ lies to the right 
				of |\Mvar|, i.e., |side_of(p) == -1|.}*/

	bool contains(const r_circle_point& p) const
	{ return !outside(p); }
	/*{\Mop     returns true if $p$ lies to the left of or on |\Mvar|.}*/

	double approximate_area() const;
	/*{\Mop     approximates the (oriented) area of the bounded region of 
	            |\Mvar|.\\
				\precond |\Mvar.kind()| is not full.}*/

public:
	static bool clean_chain(list<r_circle_segment>& segs);
	static CHECK_TYPE check_chain_for_simplicity(const list<r_circle_segment>& segs, list<r_circle_point>* crossings = 0);
	static list<r_circle_segment> reverse_chain(const list<r_circle_segment>& segs);
	static int orientation_of_weakly_simple_chain(const list<r_circle_segment>& segs);
	static int orientation_at_lex_smallest_bend
		(const r_circle_segment& s_pred, const r_circle_point& lex_smallest, const r_circle_segment& s);

protected:
	r_circle_polygon(const list<r_circle_segment>& segs, int orient) 
	{ PTR = new rep(segs, orient); }

	friend class __exportC r_circle_gen_polygon;

private:
	void init(const list<r_circle_segment>& segs, CHECK_TYPE check, RESPECT_TYPE respect_orient);
	void init(const list<rat_point>& L, CHECK_TYPE check, RESPECT_TYPE respect_orient);
};

__exportF ostream& operator<<(ostream& out, const r_circle_polygon& p);
__exportF istream& operator>>(istream& in,  r_circle_polygon& p);

/*{\Mtext
\bigskip
{\bf Iterations Macros}

{\bf forall\_vertices}($v,P$)
$\{$ ``the vertices of $P$ are successively assigned to |r_circle_point| $v$'' $\}$

{\bf forall\_segments}($s,P$)
$\{$ ``the edges of $P$ are successively assigned to the segment $s$'' $\}$ 
}*/

inline list_item
LedaForallVerticesFirstItem(const r_circle_polygon& P)
{ return P.segments().first_item(); }

inline void
LedaForallVerticesNextItem(const r_circle_polygon& P, list_item& loop_var)
{ loop_var = P.segments().next_item(loop_var); }

inline list_item
LedaForallVerticesAssign(const r_circle_polygon& P, list_item loop_var, r_circle_point& v)
{ if (loop_var) v = P.segments().inf(loop_var).source(); return loop_var; }

#if !defined(forall_vertices)
#define forall_vertices(v,P) \
LEDA_FORALL_PREAMBLE \
for(list_item vert_loop_var = LEDA_SCOPE LedaForallVerticesFirstItem(P);\
LEDA_SCOPE LedaForallVerticesAssign((P),vert_loop_var, v);\
LEDA_SCOPE LedaForallVerticesNextItem((P),vert_loop_var))
#endif

#if !defined(forall_segments)
#define forall_segments(s,P)  forall(s,(P).segments())
#endif

#if LEDA_ROOT_INCL_ID == 500268
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
