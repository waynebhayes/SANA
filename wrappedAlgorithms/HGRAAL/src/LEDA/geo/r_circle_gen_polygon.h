/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  r_circle_gen_polygon.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.5 $  $Date: 2005/04/14 10:44:56 $


#ifndef LEDA_R_CIRCLE_GEN_POLYGON_H
#define LEDA_R_CIRCLE_GEN_POLYGON_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500267
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/geo/r_circle_polygon.h>

LEDA_BEGIN_NAMESPACE

//------------------------------------------------------------------------------
// r_circle_gen_polygon:   generalized polygon with r_circle_segment edges
//------------------------------------------------------------------------------

/// r_circle_gen_polygon /////////////////////////////////////////////////////////////////////

class r_circle_gen_polygon_rep : public handle_rep {
public:
	enum KIND { EMPTY       = r_circle_polygon::EMPTY, 
		        FULL        = r_circle_polygon::FULL,
				NON_TRIVIAL = r_circle_polygon::NON_TRIVIAL };

public:
	r_circle_gen_polygon_rep(KIND k = EMPTY) : Kind(k) {}

	r_circle_gen_polygon_rep(const r_circle_polygon& p)
	{
		switch (p.kind()) {
		case EMPTY:		Kind = EMPTY; break;
		case FULL:		Kind = FULL;  break;
		default:		Kind = NON_TRIVIAL; BoundaryChains.append(p); break;
		}
	}

	r_circle_gen_polygon_rep(const list<r_circle_polygon>& chains) : BoundaryChains(chains)
	{ Kind = chains.empty() ? EMPTY : NON_TRIVIAL; }

	virtual ~r_circle_gen_polygon_rep() {}

private:
	KIND                   Kind;
	list<r_circle_polygon> BoundaryChains;

	friend class __exportC r_circle_gen_polygon;
};

class __exportC rat_gen_polygon;
class __exportC gen_polygon;

/*{\Manpage {r_circle_gen_polygon} {} {Generalized polygons with circular edges} {P}}*/

class __exportC r_circle_gen_polygon : public HANDLE_BASE(r_circle_gen_polygon_rep) {
/*{\Mdefinition
The data type |r_circle_polygon| is not closed under boolean operations, 
e.g., the set difference of a polygon $P$ and a polygon $Q$ nested in $P$ 
is a region that contains a ``hole''. Therefore we provide a generalization
called |\Mtype| which is closed under (regularized) boolean operations (see
below).

A formal definition follows:
An instance |\Mvar| of the data type |\Mtype| is a regular 
polygonal region in the plane. A regular region is an open set that is 
equal to the interior of its closure. A region is polygonal if its boundary
consists of a finite number of |r_circle_segments|.

The boundary of an |\Mtype| consists of zero or more weakly 
simple closed polygonal chains. Each such chain is represented by an 
object of type |r_circle_ploygon|.
There are two regions whose boundary is empty, namely the \emph{empty region}
and the \emph{full region}. The full region encompasses the entire plane. 
We call a region \emph{trivial} if its boundary is empty.
The boundary cycles $P_1$, $P_2$, \ldots, $P_k$ of an |\Mtype| are ordered
such that no $P_i$ is nested in a $P_j$ with $i < j$. 
}*/

public:
	typedef r_circle_gen_polygon_rep rep;
	typedef HANDLE_BASE(rep)         base;
	typedef r_circle_gen_polygon     gp_type;

private:
	rep* ptr() const { return (rep*) PTR; }

public:
/*{\Mtypes}*/

	enum KIND { EMPTY       = r_circle_polygon::EMPTY, 
		        FULL        = r_circle_polygon::FULL,
				NON_TRIVIAL = r_circle_polygon::NON_TRIVIAL };
	/*{\Menum describes the kind of the polygon: the empty set, the full plane 
	          or a non-trivial polygon.}*/

	enum CHECK_TYPE { NO_CHECK          = r_circle_polygon::NO_CHECK, 
					  SIMPLE            = r_circle_polygon::SIMPLE, 
					  WEAKLY_SIMPLE     = r_circle_polygon::WEAKLY_SIMPLE, 
					  NOT_WEAKLY_SIMPLE = r_circle_polygon::NOT_WEAKLY_SIMPLE };
	/*{\Menum used to specify which checks should be applied and also describes
	          the outcome of a simplicity check.}*/

	enum RESPECT_TYPE { DISREGARD_ORIENTATION = r_circle_polygon::DISREGARD_ORIENTATION, 
						RESPECT_ORIENTATION   = r_circle_polygon::RESPECT_ORIENTATION };
	/*{\Menum used in contructors to specify whether to force a positive
	          orientation for the constructed object (|DISREGARD_ORIENTATION|)
			  or to keep the orientation of the input (|RESPECT_ORIENTATION|).}*/

	enum BOOL_OP { UNITE, INTERSECT, DIFF, SYMDIFF };

public:
/*{\Mcreation}*/

	r_circle_gen_polygon() { PTR = new rep; }
	/*{\Mcreate creates an empty polygon |\Mvar|.}*/

	r_circle_gen_polygon(KIND k);
	/*{\Mcreate creates a polygon |\Mvar| of kind |k|, where |k| is either 
	            |EMPTY| or |FULL|.}*/

	r_circle_gen_polygon(const list<r_circle_segment>& seg_chain, 
						 CHECK_TYPE check = WEAKLY_SIMPLE, RESPECT_TYPE respect_orient = RESPECT_ORIENTATION)
	{ PTR = new rep(r_circle_polygon(seg_chain, r_circle_polygon::CHECK_TYPE(check), r_circle_polygon::RESPECT_TYPE(respect_orient))); }
	/*{\Mcreate creates a polygon |\Mvar| from a single closed chain of 
	            segments.}*/

	r_circle_gen_polygon(const r_circle_polygon& Q,
						 CHECK_TYPE check = NO_CHECK, RESPECT_TYPE respect_orient = RESPECT_ORIENTATION);
	/*{\Mcreate converts an |r_circle_polygon| $Q$ to an |\Mtype| |\Mvar|.}*/

	r_circle_gen_polygon(const list<rat_point>& L,
						 CHECK_TYPE check = NO_CHECK, RESPECT_TYPE respect_orient = RESPECT_ORIENTATION)
	{ PTR = new rep(r_circle_polygon(L, r_circle_polygon::CHECK_TYPE(check), r_circle_polygon::RESPECT_TYPE(respect_orient))); }
	/*{\Mcreate creates a polygon |\Mvar| with straight line edges from a 
	            list |L| of vertices.}*/

	r_circle_gen_polygon(const list<r_circle_polygon>& polys, 
						 CHECK_TYPE check = NO_CHECK, RESPECT_TYPE respect_orient = RESPECT_ORIENTATION)
	{ init(polys, check, respect_orient); }
	/*{\Mcreate introduces a variable |\Mvar| of type |\Mname|.
	            |\Mvar| is initialized to the polygon with boundary 
				representation |polys|.\\
	            \precond |polys| must be a boundary representation.}*/

	r_circle_gen_polygon(const rat_gen_polygon& Q,
						 CHECK_TYPE check = NO_CHECK, RESPECT_TYPE respect_orient = RESPECT_ORIENTATION);
	/*{\Mcreate converts a |rat_gen_polygon| $Q$ to an |\Mtype| |\Mvar|.}*/

	r_circle_gen_polygon(const gen_polygon& Q, 
						 CHECK_TYPE check = NO_CHECK, RESPECT_TYPE respect_orient = RESPECT_ORIENTATION, 
						 int prec = rat_point::default_precision);
	/*{\Mcreate converts the (floating point) |gen_polygon| $Q$ to an |\Mtype|.
	            |\Mvar| is initialized to a rational approximation of $Q$ of 
				coordinates with denominator at most |prec|. 
				If |prec| is zero, the implementation chooses |prec| large 
				enough such that there is no loss of precision in the 
				conversion.}*/

	r_circle_gen_polygon(const rat_circle& circ, RESPECT_TYPE respect_orient = RESPECT_ORIENTATION)
	{ PTR = new rep(r_circle_polygon(circ, r_circle_polygon::RESPECT_TYPE(respect_orient))); }
	/*{\Mcreate creates a polygon |\Mvar| whose boundary is the 
	            circle |circ|.}*/

	r_circle_gen_polygon(const r_circle_gen_polygon& Q) : base(Q) {}

	const r_circle_gen_polygon& operator=(const r_circle_gen_polygon& Q)
	{ base::operator=(Q); return *this; }

/*{\Moperations}*/

	KIND kind() const { return KIND(ptr()->Kind); }
	/*{\Mop     returns the kind of |\Mvar|.}*/

	/*{\Moptions nextwarning=no}*/
	bool is_trivial() const { return kind() != NON_TRIVIAL; }
	bool trivial() const { return is_trivial(); } // for compatibility with GEN_POLYGON
	/*{\Mop     returns true iff |\Mvar| is trivial.}*/

	/*{\Moptions nextwarning=no}*/
	bool is_empty() const { return kind() == EMPTY; }
	bool empty() const { return is_empty(); } // for compatibility with GEN_POLYGON
	/*{\Mop     returns true iff |\Mvar| is empty.}*/

	/*{\Moptions nextwarning=no}*/
	bool is_full() const { return kind() == FULL; }
	bool full() const { return is_full(); } // for compatibility with GEN_POLYGON
	/*{\Mop     returns true iff |\Mvar| is full.}*/

	void normalize();
	/*{\Mop     simplifies the representation by calling
	            |c.normalize()| for every polygonal chain |c| of |\Mvar|.}*/

	bool is_simple() const
	{ return r_circle_polygon::check_chain_for_simplicity(segments()) == r_circle_polygon::SIMPLE; }
	/*{\Mop   tests whether |\Mvar| is simple or not.}*/

	bool is_weakly_simple() const
	{ return r_circle_polygon::check_chain_for_simplicity(segments()) != r_circle_polygon::NOT_WEAKLY_SIMPLE; }
	/*{\Mop   tests whether |\Mvar| is weakly simple or not.}*/

	static bool check_representation(const list<r_circle_polygon>& polys, CHECK_TYPE check = WEAKLY_SIMPLE);
	/*{\Mstatic checks whether |polys| is a boundary representation.
	            Currently the nesting order is not checked, 
				we check only for (weak) simplicity.}*/

	bool check_representation() const
	{ return check_representation(polygons()); }
	/*{\Mop   checks the representation of |\Mvar| (see above).}*/

	bool is_convex() const;
	/*{\Mop     returns true iff |\Mvar| is convex.}*/

	int size() const;
	/*{\Mop     returns the size of |\Mvar|, i.e.~the number of segments in 
	            its boundary representation.}*/

	const list<r_circle_polygon>& polygons() const { return ptr()->BoundaryChains; }
	/*{\Mop     returns the boundary representation of |\Mvar|.}*/

	/*{\Moptions nextwarning=no}*/
	list<r_circle_segment> segments() const;
	list<r_circle_segment> edges() const { return segments(); }
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

	r_circle_gen_polygon translate(rational dx, rational dy) const;
	/*{\Mopl    returns |\Mvar| translated by vector $(dx,dy)$.}*/

	r_circle_gen_polygon translate(const rat_vector& v) const { return translate(v[0], v[1]); }
	/*{\Mop     returns |\Mvar| translated by vector $v$.}*/

	r_circle_gen_polygon operator+(const rat_vector& v) const { return translate(v); }
	/*{\Mbinop  returns |\Mvar| translated by vector $v$.}*/

	r_circle_gen_polygon operator-(const rat_vector& v) const { return translate(-v); }
	/*{\Mbinop  returns |\Mvar| translated by vector $-v$.}*/

	r_circle_gen_polygon rotate90(const rat_point& q, int i=1) const;
	/*{\Mopl    returns |\Mvar| rotated about $q$ by an angle of $i\times 90$ 
				degrees. If $i > 0$ the rotation is counter-clockwise otherwise
				it is clockwise. }*/

	r_circle_gen_polygon reflect(const rat_point& p, const rat_point& q) const;
	/*{\Mop     returns |\Mvar| reflected  across the straight line passing
				through $p$ and $q$.}*/

	r_circle_gen_polygon reflect(const rat_point& p) const;
	/*{\Mop     returns |\Mvar| reflected  across point $p$.}*/


	real sqr_dist(const real_point& p) const;
	/*{\Mop computes the squared Euclidean distance between the boundary of 
	        |\Mvar| and $p$. (If |\Mvar| is zero, the result is zero.)}*/

	real dist(const real_point& p) const { return sqrt(sqr_dist(p)); }
	/*{\Mop computes the Euclidean distance between the boundary of 
	        |\Mvar| and $p$. (If |\Mvar| is zero, the result is zero.)}*/


	r_circle_gen_polygon complement() const;
	/*{\Mopl    returns the complement of |\Mvar|.}*/

	r_circle_gen_polygon eliminate_cocircular_vertices() const;
	/*{\Mopl    returns a copy of |\Mvar| without cocircular vertices.}*/


	bool is_r_circle_polygon() const { return polygons().size() <= 1; }
	/*{\Mopl    checks if the boundary of |\Mvar| consists of at most one chain.}*/

	r_circle_polygon to_r_circle_polygon() const;
	/*{\Mopl    converts |\Mvar| to an |r_circle_polygon|.\\ 
	            \precond |is_r_circle_polygon| is true.}*/


	bool is_rat_gen_polygon() const;
	/*{\Mopl    returns whether |\Mvar| can be converted to a |rat_polygon|.}*/

	rat_gen_polygon to_rat_gen_polygon() const;
	/*{\Mopl    converts |\Mvar| to a |rat_gen_polygon|.\\ 
	            \precond |is_rat_gen_polygon| is true.}*/

	gen_polygon to_float() const;
	/*{\Mopl    computes a floating point approximation of |\Mvar| with 
	            straight line segments.\\ 
	            \precond |is_rat_gen_polygon| is true.}*/

	bool is_rat_circle() const;
	/*{\Mopl    returns whether |\Mvar| can be converted to a |rat_circle|.}*/

	rat_circle to_rat_circle() const;
	/*{\Mopl    converts |\Mvar| to a |rat_circle|.\\ 
	            \precond |is_rat_circle| is true.}*/

/*{\Mtext 
\medskip
{\bf All functions below assume that |\Mvar| is weakly simple.}
\medskip
}*/

	int orientation() const
	{ return is_trivial() ? 0 : polygons().head().orientation(); }
	/*{\Mop     returns the orientation of |\Mvar|. }*/

	int         side_of(const r_circle_point& p) const;
	/*{\Mop     returns $+1$ if $p$ lies to the left of |\Mvar|, $0$ if 
				$p$ lies on |\Mvar|, and $-1$ if $p$ lies to the 
				right of |\Mvar|.}*/ 

	region_kind region_of(const r_circle_point& p) const;
	/*{\Mop     returns BOUNDED\_REGION if $p$ lies in the 
				bounded region of |\Mvar|, returns ON\_REGION if $p$ lies on 
				|\Mvar|, and returns UNBOUNDED\_REGION if $p$ lies in the 
				unbounded region. The bounded region of the full polygon is the entire plane. }*/

	bool        inside(const r_circle_point& p) const 
	            { return side_of(p) == 1; }
	/*{\Mop     returns true if $p$ lies to the left of |\Mvar|, i.e.,
				|side_of(p) == +1|. }*/

	bool        on_boundary(const r_circle_point& p) const  
				{ return side_of(p) == 0; }
	/*{\Mop     returns true if $p$ lies on |\Mvar|, i.e., 
				|side_of(p) == 0|. }*/

	bool        outside(const r_circle_point& p) const 
				{ return side_of(p) == -1; }
	/*{\Mop     returns true if $p$ lies to the right of |\Mvar|, i.e., 
				|side_of(p) == -1|.}*/

	bool        contains(const r_circle_point& p) const
				{ return !outside(p); }
	/*{\Mop     returns true if $p$ lies to the left of or on |\Mvar|.}*/


	double approximate_area() const;
	/*{\Mop     approximates the (oriented) area of the bounded region of 
	            |\Mvar|.\\
				\precond |\Mvar.kind()| is not full.}*/

/*{\Mtext \newcommand{\regp}{\mathop {\rm reg}} 
All binary boolean operations are regularized, i.e., the result 
$R$ of the standard boolean operation is replaced by the interior 
of the closure of $R$. We use $\regp X$ to denote the regularization 
of a set $X$.
}*/

	/*{\Moptions nextwarning=no}*/
	r_circle_gen_polygon unite(const r_circle_gen_polygon& Q) const
	{ return bool_operation(UNITE, *this, Q); }
	/*{\Mopl    returns $\regp (P \cup Q)$. }*/

	/*{\Moptions nextwarning=no}*/
	r_circle_gen_polygon intersection(const r_circle_gen_polygon& Q) const
	{ return bool_operation(INTERSECT, *this, Q); }
	/*{\Mopl    returns $\regp(P \cap Q)$.}*/

	/*{\Moptions nextwarning=no}*/
	r_circle_gen_polygon diff(const r_circle_gen_polygon& Q) const
	{ return bool_operation(DIFF, *this, Q); }
	/*{\Mopl    returns $\regp(P \setminus Q)$.}*/

	/*{\Moptions nextwarning=no}*/
	r_circle_gen_polygon sym_diff(const r_circle_gen_polygon& Q) const
	{ return bool_operation(SYMDIFF, *this, Q); }
	/*{\Mopl    returns $\regp( (P\cup Q) - (P\cap Q))$.}*/

	static r_circle_gen_polygon unite(const list<r_circle_gen_polygon>& L);
	/*{\Mstatic returns the (regularized) union of all polygons in $L$.}*/

	bool operator==(const r_circle_gen_polygon& Q) const;
	bool operator!=(const r_circle_gen_polygon& Q) const { return ! operator==(Q); }

private:
	void init(const list<r_circle_polygon>& polys, CHECK_TYPE check, RESPECT_TYPE respect_orient);

private: // helpers for boolean operatiosn
	static gp_type bool_operation(BOOL_OP op, const gp_type& gp1, const gp_type& gp2);
	static gp_type bool_op_trivial_case(BOOL_OP op, const gp_type& gp1, const gp_type& gp2);
	static void    bool_op_construct_map(const gp_type& gp1, const gp_type& gp2, GRAPH<r_circle_point, r_circle_segment>& G, edge_array<unsigned>& label);
	static void    bool_op_mark_relevant_edges(BOOL_OP op, const GRAPH<r_circle_point, r_circle_segment>& G, const edge_array<unsigned>& label, edge_array<bool>& relevant);
	static bool    bool_op_simplify_graph(GRAPH<r_circle_point, r_circle_segment>& G, const edge_array<bool>& relevant);
	static gp_type bool_op_extract_result(GRAPH<r_circle_point, r_circle_segment>& G, const edge_array<bool>& relevant);
};

__exportF ostream& operator<<(ostream& out, const r_circle_gen_polygon& gp);
__exportF istream& operator>>(istream& in,  r_circle_gen_polygon& gp);

/*{\Mtext
\bigskip
{\bf Iterations Macros}

{\bf forall\_polygons}($p,P$)       
$\{$ ``the boundary polygons of $P$ are successively assigned to 
|r_circle_polygon| $p$'' $\}$ 
}*/

#if !defined(forall_polygons)
#define forall_polygons(v,P)  forall(v,(P).polygons())
#endif


#if LEDA_ROOT_INCL_ID == 500267
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif


LEDA_END_NAMESPACE

#endif
