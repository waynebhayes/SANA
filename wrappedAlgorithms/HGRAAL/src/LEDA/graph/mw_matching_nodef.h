/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  mw_matching_nodef.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:45:10 $


LEDA_BEGIN_NAMESPACE

// Some compilers still do not support default arguments in 
// function templates. This version of mw_matching.h is using
// overloading instead.


template<class NT> 
list<edge> MAX_WEIGHT_MATCHING_T(const graph &G, 
                                 const edge_array<NT> &w, 
                                 bool check, int heur);


template<class NT> inline
list<edge> MAX_WEIGHT_MATCHING_T(const graph &G, 
                                 const edge_array<NT> &w) 
{ 
  return MAX_WEIGHT_MATCHING_T(G,w,true,2); 
}


template<class NT>
list<edge> MAX_WEIGHT_MATCHING_T(const graph &G, 
                                 const edge_array<NT> &w, 
				 node_array<NT> &pot, 
				 array<two_tuple<NT, int> > &BT, 
				 node_array<int> &b,
                                 bool check, int heur);

template<class NT> inline
list<edge> MAX_WEIGHT_MATCHING_T(const graph &G, 
                                 const edge_array<NT> &w, 
		                 node_array<NT> &pot, 
				 array<two_tuple<NT, int> > &BT, 
				 node_array<int> &b)
{ 
  return MAX_WEIGHT_MATCHING_T(G,w,pot,BT,b,true,2); 
}


template<class NT>
bool CHECK_MAX_WEIGHT_MATCHING_T(const graph &G,
                                 const edge_array<NT> &w,
				 const list<edge> &M,
				 const node_array<NT> &pot,
				 const array<two_tuple<NT, int> > &BT,
				 const node_array<int> &b);





template<class NT>
list<edge> MAX_WEIGHT_PERFECT_MATCHING_T(const graph &G, 
                                         const edge_array<NT> &w, 
					 bool check, int heur);

template<class NT> inline
list<edge> MAX_WEIGHT_PERFECT_MATCHING_T(const graph &G, 
                                         const edge_array<NT> &w) 
{ 
  return MAX_WEIGHT_PERFECT_MATCHING_T(G,w,true,2); 
}




template<class NT> 
list<edge> MAX_WEIGHT_PERFECT_MATCHING_T(const graph &G, 
                                         const edge_array<NT> &w, 
					 node_array<NT> &pot, 
					 array<two_tuple<NT, int> > &BT, 
					 node_array<int> &b,
					 bool check, int heur);

template<class NT>  inline
list<edge> MAX_WEIGHT_PERFECT_MATCHING_T(const graph &G, 
                                         const edge_array<NT> &w, 
        				 node_array<NT> &pot, 
        				 array<two_tuple<NT, int> > &BT, 
        				 node_array<int> &b)
{ 
  return MAX_WEIGHT_MATCHING_T(G,w,pot,BT,b,true,2); 
}




template<class NT>
bool CHECK_MAX_WEIGHT_PERFECT_MATCHING_T(const graph &G,
                                         const edge_array<NT> &w,
					 const list<edge> &M,
					 const node_array<NT> &pot,
					 const array<two_tuple<NT, int> > &BT,
					 const node_array<int> &b);





template<class NT> 
list<edge> MIN_WEIGHT_PERFECT_MATCHING_T(const graph &G, 
                                         const edge_array<NT> &w, 
                                         bool check, int heur);

template<class NT> inline
list<edge> MIN_WEIGHT_PERFECT_MATCHING_T(const graph &G, 
                                         const edge_array<NT> &w) 
{ 
  return MIN_WEIGHT_PERFECT_MATCHING_T(G,w,true,2); 
}




template<class NT> 
list<edge> MIN_WEIGHT_PERFECT_MATCHING_T(const graph &G, 
                                         const edge_array<NT> &w, 
				         node_array<NT> &pot, 
				         array<two_tuple<NT, int> > &BT, 
				         node_array<int> &b,
                                         bool check, int heur);

template<class NT>  inline
list<edge> MIN_WEIGHT_PERFECT_MATCHING_T(const graph &G, 
                                         const edge_array<NT> &w, 
        				 node_array<NT> &pot, 
        				 array<two_tuple<NT, int> > &BT, 
        				 node_array<int> &b)
{ 
  return MIN_WEIGHT_MATCHING_T(G,w,pot,BT,b,true,2); 
}







template<class NT>
bool CHECK_MIN_WEIGHT_PERFECT_MATCHING_T(const graph &G,
                                         const edge_array<NT> &w,
					 const list<edge> &M,
					 const node_array<NT> &pot,
					 const array<two_tuple<NT, int> > &BT,
					 const node_array<int> &b);

template<class NT> 
bool CHECK_WEIGHTS_T(const graph &G, edge_array<NT> &w, bool perfect);


#include <LEDA/graph/instant/mw_matching.h>

LEDA_END_NAMESPACE


