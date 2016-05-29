/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  curve_sweep.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.4 $  $Date: 2005/04/14 10:44:50 $

#ifndef LEDA_CURVE_SWEEP_H
#define LEDA_CURVE_SWEEP_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500800
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/graph/graph.h>
#include <LEDA/core/map.h>
#include <LEDA/core/p_queue.h>
#include <LEDA/core/sortseq.h>

// switches CHECK on/off
//#define LEDA_CURVE_SWEEP_CHECK

// switches TIME on/off
//#define LEDA_CURVE_SWEEP_TIME

// switches TRACE on/off
//#define LEDA_CURVE_SWEEP_TRACE
//#define LEDA_CURVE_SWEEP_TRACE_BUNDLE
//#define LEDA_CURVE_SWEEP_TRACE_INTERSECTION

// switches visual (animated) debugging on or off
//#define LEDA_CURVE_SWEEP_VISUAL_DEBUG
//#define LEDA_CURVE_SWEEP_COMPACTIFICATION_VISUAL_DEBUG

// check, trace and visual-debug only if LEDA_DEBUG is switched on ...
#if !defined(LEDA_DEBUG)
#undef LEDA_CURVE_SWEEP_CHECK
#undef LEDA_CURVE_SWEEP_TRACE
#undef LEDA_CURVE_SWEEP_TRACE_BUNDLE
#undef LEDA_CURVE_SWEEP_TRACE_INTERSECTION
#undef LEDA_CURVE_SWEEP_VISUAL_DEBUG
#undef LEDA_CURVE_SWEEP_COMPACTIFICATION_VISUAL_DEBUG
#endif

#ifdef LEDA_INSTALL
#undef LEDA_CURVE_SWEEP_TIME
#endif

// visual debugging needs at least LEDA_CURVE_SWEEP_CHECK
#if defined(LEDA_CURVE_SWEEP_VISUAL_DEBUG) && !defined(LEDA_CURVE_SWEEP_CHECK)
#if !defined(LEDA_CURVE_SWEEP_TRACE)
// if CHECK is not defined, we enable the full trace
#define LEDA_CURVE_SWEEP_TRACE
#endif
#endif

// if we trace, we should check ...
#if defined(LEDA_CURVE_SWEEP_TRACE) && !defined(LEDA_CURVE_SWEEP_CHECK)
#define LEDA_CURVE_SWEEP_CHECK
#endif

// if we trace intersections, we should trace the bundle, too ...
#if defined(LEDA_CURVE_SWEEP_TRACE_INTERSECTION) && !defined(LEDA_CURVE_SWEEP_TRACE_BUNDLE)
#define LEDA_CURVE_SWEEP_TRACE_BUNDLE
#endif

LEDA_BEGIN_NAMESPACE

/// curve_sweep_cmp /////////////////////////////////////////////////////////////////////////

// compare object for the CURVEs intersecting the current sweepline

template <class traits>
class curve_sweep_cmp : public leda_cmp_base<typename traits::CURVE> {
public:
	typedef typename traits::CURVE   CURVE;
	typedef typename traits::CRV_PNT CRV_PNT;

	int operator()(const CURVE& c1, const CURVE& c2) const 
	{ return traits::compare_intersections_with_sweepline_at(SweepPos, c1, c2); }
	void set_position(const CRV_PNT& pos) { SweepPos = pos; }
	CRV_PNT get_position() const { return SweepPos; }
private:
	CRV_PNT SweepPos;
};

/// curve_sweep /////////////////////////////////////////////////////////////////////////////

template <class traits>
class curve_sweep : public traits {
public:
	typedef traits                   TRAITS;
	typedef typename TRAITS::CURVE   CURVE;
	typedef typename TRAITS::CRV_PNT CRV_PNT;
	typedef curve_sweep_cmp<TRAITS>  CRV_COMPARE;

public:
	curve_sweep();

	void set_embedding_flag(bool flag = true) { EmbedGraph = flag; }
	bool get_embedding_flag() const { return EmbedGraph; }

	void set_compactification_flag(bool flag = true) { CompactifyGraph = flag; }
	bool get_compactification_flag() const { return CompactifyGraph; }
		// remove nodes that are no real end point or proper intersection point
		// of the original curves (introduced by make_x_monotonous)

	void run(const list<CURVE>& Cs, GRAPH<CRV_PNT,CURVE>& G);

	void run(const list<CURVE>& Cs, GRAPH<CRV_PNT,CURVE>& G, edge_array<edge>& corresp_edge);
		// e1,e2 correspond iff (1) e1 and e2 are labelled with identical curves
	    //                  and (2) !EmbedGraph => source(e1) == target(e2)
		//                           EmbedGraph => source(e1) == source(e2) && target(e1) != target(e2)

	void run_brute_force(const list<CURVE>& Cs, GRAPH<CRV_PNT,CURVE>& G);

private:
	void do_run(const list<CURVE>& Cs);
	void initialize(const list<CURVE>& Cs);
	void sweep();
	void handle_ending_and_passing_curves();
	void handle_starting_curves();
	void post_process();

	void compute_corresponding_edges(edge_array<edge>& corresp_edge);
	void compute_corresponding_edges_for(node n, edge_array<edge>& corresp_edge);

	void compactify_simple_graph(); // i.e. not embedded
	void compactify_embedding();

	GRAPH<CRV_PNT,CURVE>& get_graph() { return *GraphPtr; }

protected:
	const GRAPH<CRV_PNT,CURVE>& get_graph() const { return *GraphPtr; }
	CRV_PNT get_sweep_position() const { return SweepPos; }

	seq_item nil_marker() const { return seq_item(0); }
	seq_item overlapping_marker() const { return seq_item(0)+1; }

	bool overlaps_succ(seq_item sit) const { return YStructure.inf(sit) == overlapping_marker(); }

	bool touches_succ_at_sweep_pos(seq_item sit)
	{ return TRAITS::A_touches_B_at_sweeppos_given_that_A_leq_B(SweepPos, YStructure.key(sit), YStructure.key(YStructure.succ(sit))); }

	void mark_as_touching_at_sweep_pos(seq_item sit_A, seq_item sit_B)
	{ TRAITS::mark_as_touching_at_p(SweepPos, YStructure.key(sit_A), YStructure.key(sit_B)); }

private:
	GRAPH<CRV_PNT,CURVE>*      GraphPtr;
	edge_array<edge>*          PtrToCorrespEdgeArray;
	bool                       EmbedGraph;
	bool                       CompactifyGraph;

	map<CURVE,CURVE>           OriginalCurve;
	map<CURVE,node>            LastNode;
	map<CURVE,edge>            LastEdge; // for embedding

	CRV_PNT                    SweepPos;
	node                       NodeAtSweepPos;
	CRV_COMPARE                SweepCompare;
	p_queue<CRV_PNT, CURVE>    CurveQueue;
	sortseq<CRV_PNT, seq_item> XStructure;
	sortseq<CURVE,   seq_item> YStructure;
	CURVE                      NextCurve;
	seq_item                   Event;
	seq_item                   ItemBelowSweepPos, ItemAboveSweepPos;

#if defined(LEDA_CURVE_SWEEP_CHECK) || defined(LEDA_CURVE_SWEEP_TRACE)
protected:
	bool has_nil_marker(seq_item sit) const { return YStructure.inf(sit) == nil_marker(); }
	bool is_marked_with_event(seq_item sit) const { return !has_nil_marker(sit) && !overlaps_succ(sit); }

	virtual bool check(int accuracy = 0) const;
	virtual bool check_invariants(int accuracy = 0) const;
	virtual void check_report_error(seq_item yit, string msg) const;

	virtual bool confirm(const string& msg) const { return Yes(msg) != 0; }
	virtual void dump_YStructure(const string& msg = string(), ostream& out = cout) const;

	string trc_yitem_inf_to_str(seq_item yit) const;
	string trc_crv_id_to_str(const CURVE& crv) const;

	virtual void trc_begin_sweep(const list<CURVE>& orig_curves) const {}
	virtual void trc_end_sweep() const {}
	virtual void trc_new_sweep_pos(const CRV_PNT& new_pos, node node_at_new_pos) const;
	virtual void trc_start_curve(const CURVE& crv) const;
	virtual void trc_end_curve(const CURVE& crv) const;
	virtual void trc_passing_curve(const CURVE& crv) const;
	virtual void trc_new_edge(edge e) const;
	virtual void trc_new_edge_stub(edge e) const;
	virtual void trc_check_bundle_for_intersections(seq_item below, seq_item above) const;
	virtual void trc_check_for_intersection(seq_item sit) const;
	virtual void trc_found_intersection(seq_item sit, const CRV_PNT& inter) const;

	const sortseq<CURVE,seq_item>& get_YStructure() const { return YStructure; }
	const sortseq<CRV_PNT,seq_item>& get_XStructure() const { return XStructure; }
#endif
};

/// curve_sweep_trc_win /////////////////////////////////////////////////////////////////////

#if defined(LEDA_CURVE_SWEEP_VISUAL_DEBUG)

class __exportC window;
class __exportC panel;

template <class traits>
class curve_sweep_trc_win : public curve_sweep<traits> {
public:
	typedef curve_sweep<traits> base;

public:
	curve_sweep_trc_win(window* trc_win = 0);
	~curve_sweep_trc_win();

protected:
	window* win() const { return TraceWin; }
	panel* mpanel() const { return MessagePanel; }
	void wait(string msg) const;
	void draw_curve(const CURVE& crv, int col, bool xor_mode = false) const;
	void draw_pos(const CRV_PNT& p, int col, bool xor_mode = false) const;
	void highlight(const CURVE& crv, string msg, int col = 2) const;

	virtual void check_report_error(seq_item yit, string msg) const;
	virtual bool confirm(const string& msg) const { return mpanel() && mpanel()->confirm(msg); }
	virtual void dump_YStructure(const string& msg = string(), ostream& out = cout) const;

	virtual void trc_begin_sweep(const list<CURVE>& orig_curves) const;
//	virtual void trc_end_sweep() const;
	virtual void trc_new_sweep_pos(const CRV_PNT& new_pos, node node_at_new_pos) const;
	virtual void trc_start_curve(const CURVE& crv) const;
	virtual void trc_end_curve(const CURVE& crv) const;
	virtual void trc_passing_curve(const CURVE& crv) const;
	virtual void trc_new_edge(edge e) const;
	virtual void trc_check_bundle_for_intersections(seq_item below, seq_item above) const;

private:
	window* TraceWin;
	panel*  MessagePanel;
};

#endif // #if defined(LEDA_CURVE_SWEEP_VISUAL_DEBUG)

#ifdef LEDA_CURVE_SWEEP_COMPACTIFICATION_VISUAL_DEBUG
extern window* TraceWinForCompatification;
#endif

/// test & debugging ////////////////////////////////////////////////////////////////////////

#if defined(LEDA_CURVE_SWEEP_TIME)
#define TIME(CODE) CODE;
#else
#define TIME(CODE) ((void)0)
#endif

#if defined(LEDA_CURVE_SWEEP_CHECK)
#define CHECK(CODE) CODE;
#else
#define CHECK(CODE) ((void)0)
#endif

#if defined(LEDA_CURVE_SWEEP_TRACE)
#define TRACE(CODE) CODE;
#else
#define TRACE(CODE) ((void)0)
#endif


#if LEDA_ROOT_INCL_ID == 500800
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

#endif
