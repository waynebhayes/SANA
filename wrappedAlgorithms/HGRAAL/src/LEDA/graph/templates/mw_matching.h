/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  mw_matching.t
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.7 $  $Date: 2005/04/14 14:20:24 $

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500365
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/graph/mw_matching.h>

#include <LEDA/core/array.h>
#include <LEDA/core/tuple.h>
#include <LEDA/core/slist.h>
#include <LEDA/core/map.h>
#include <LEDA/graph/graph.h>
#include <LEDA/graph/graph_misc.h>
#include <LEDA/graph/graph_alg.h>
#include <LEDA/core/p_queue.h>
#include <LEDA/graph/node_slist.h>
#include <LEDA/graph/node_pq.h>

#include <LEDA/core/map.h>
#include <LEDA/core/h_array.h>

#include <LEDA/internal/std/stdio.h>
#include <LEDA/internal/std/limits.h>
#include <LEDA/internal/std/float.h>
#include <LEDA/system/assert.h>

//#define _SST_APPROACH

// initial size of the PQs (i.e. bin. heaps) stored for each node/tree
#define MWM_INIT_PQ_SZ 1

LEDA_BEGIN_NAMESPACE

inline char   get_max_val(const char*)   { return CHAR_MAX;  }
inline short  get_max_val(const short*)  { return SHRT_MAX;  }
inline int    get_max_val(const int*)    { return INT_MAX;   }
inline long   get_max_val(const long*)   { return LONG_MAX;  }
inline float  get_max_val(const float*)  { return FLT_MAX; }
inline double get_max_val(const double*) { return DBL_MAX; }

template<class NT> inline NT get_max_val(const NT*) 
{ error_handler(1, "Sorry, `max_val' not specified for that type."); }

#define MAX_VALUE(T) get_max_val((T*)0)


typedef enum {even, odd, unlabeled} LABEL;


// ----- #include "checker.t"

/*
	Conditions checked:
	1) non-negativity of reduced costs: pi[e=(u,v)] := pot[u] + pot[v] - w[e] >= 0
	2) half-integrality of reduced costs: pi[e] % 2 == 0
	3) mate describes a matching
*/
template<class NT> 
__temp_func_inline
void check_feasibility(const graph &G, 
		       const edge_array<NT> &w, 
		       node_array<NT> &pot,
		       node_array<node> &mate, 
		       bool perfect ) {

  edge e;
  node u, v;

  forall_edges(e, G) {
    u = source(e);
    v = target(e);   
    assert(pot[u] + pot[v] >= w[e]);
    assert(((pot[u] + pot[v] - w[e]) % 2) == 0);
  }

  if (!perfect) forall_nodes(u, G) assert(pot[u] >= 0);

  forall_edges(e, G) {
    u = source(e);
    v = target(e);
    
    if (mate[source(e)]) 
      assert(mate[mate[source(e)]] == source(e));
    if (mate[target(e)])
      assert(mate[mate[target(e)]] == target(e));
  }

  node_array<bool> matched(G, false);

  forall_edges(e, G) {
    u = source(e);
    v = target(e);

    if (mate[u] && (mate[u] == v) &&
	mate[v] && (mate[v] == u)) {

      assert(!matched[u]);
      assert(!matched[v]);
    
      matched[u] = matched[v] = true;
    }
  }

  forall_nodes(u, G) assert( (mate[u] != nil) == matched[u] );
}

// ----- #include "conc_pq_tree.h"


// ----------------------------------------------------------------------
//  Class `conc_pq_node'
// ----------------------------------------------------------------------

template <int a, int b>
class conc_pq_tree;

template <int a, int b>
class conc_pq_node {  

  friend class conc_pq_tree<a,b>;
/*
  friend inline conc_pq_node<a,b>* my_root(conc_pq_node<a,b>* p);
  friend inline conc_pq_tree<a,b>* my_tree(conc_pq_node<a,b>* p);
*/

  conc_pq_tree<a,b>* tree;     // pointer to the tree containing this node

  int height;      // height (!depth) of tree rooted at this
  int leaves;      // number of leaves in tree rooted at this
  int children;    // number of children

  GenPtr key;      // key  stored in conc_pq_node
  GenPtr inf;      // info stored in conc_pq_node

  
  conc_pq_node<a,b> *succ;   // pointer to succ leaf
  conc_pq_node<a,b> *pred;   // pointer to pred leaf
  conc_pq_node<a,b> *left;   // pointer to left sibbling
  conc_pq_node<a,b> *right;  // pointer to right sibbling
  conc_pq_node<a,b> *parent; // pointer to parent node
  conc_pq_node<a,b> *child;  // pointer to leftmost child


public:

  conc_pq_node(GenPtr k, GenPtr i, conc_pq_tree<a,b>* p) {
    tree = p;
    height = 0;
    leaves = 1;
    children = 0;
    key  = k;
    inf  = i;
    succ = pred  = nil;
    left = right = this;
    parent = child = nil;
  }

  ~conc_pq_node() {}

  static inline conc_pq_node<a,b>* get_root(conc_pq_node<a,b> *p);
  static inline conc_pq_tree<a,b>* get_tree(conc_pq_node<a,b> *p);
  static inline GenPtr             get_owner(conc_pq_node<a,b>* it); 

  LEDA_MEMORY(conc_pq_node);

};



// ----------------------------------------------------------------------
//  Declaration of Class `conc_pq_tree'
// ----------------------------------------------------------------------

template <int a = 2, int b = 16>
class conc_pq_tree {

private:

  // list of leaves
  conc_pq_node<a,b> *head_leaf, *tail_leaf;
  
//  GenPtr tree_inf;      // information associated with the tree 
                        // (not realized yet)

  conc_pq_node<a,b> *root;   // pointer to the root node 
  conc_pq_node<a,b> *minptr; // pointer to the min. leaf


  conc_pq_node<a,b>* new_conc_pq_node(GenPtr k, GenPtr i) 
  { return new conc_pq_node<a,b>(k, i, this); }

  conc_pq_node<a,b>* locate_min(conc_pq_node<a,b> *p) const;

  void decrease_prio(conc_pq_node<a,b> *p, GenPtr k);
  conc_pq_node<a,b>* increase_prio(conc_pq_node<a,b> *p, GenPtr k);

  conc_pq_node<a,b>* split_node(conc_pq_node<a,b>* p);
  void delete_node(conc_pq_node<a,b> *p); 

  void add_leaves(conc_pq_node<a,b> *p, int leaves);
  void print_tree(conc_pq_node<a,b> *local_root, int blancs) const; 
  void traverse_tree(conc_pq_node<a,b>* cur, GenPtr k = nil);

  conc_pq_node<a,b>* conc(conc_pq_node<a,b>* r1, conc_pq_node<a,b>* r2);


  virtual int cmp(GenPtr, GenPtr) const { return 0; }
  
  virtual void clear_key(GenPtr&) const {}
  virtual void clear_inf(GenPtr&) const {}
  virtual void copy_key(GenPtr&)  const {}
  virtual void copy_inf(GenPtr&)  const {}
  virtual void print_key(GenPtr&, ostream &out = cout)  const {}
  virtual void print_inf(GenPtr&, ostream &out = cout)  const {}

  virtual int key_type_id() const { return UNKNOWN_TYPE_ID; }

  conc_pq_tree(const conc_pq_tree<a,b>& pq);                        // not realized
  conc_pq_tree& operator=(const conc_pq_tree<a,b>& pq);             // not realized

public:

  GenPtr infinity;                // max value of key type

  typedef conc_pq_node<a,b>* item;

  conc_pq_tree();  
  conc_pq_tree(GenPtr p, GenPtr i);

  conc_pq_node<a,b>* init(GenPtr p, GenPtr i);

  virtual ~conc_pq_tree() { clear(); }

  const GenPtr& key(conc_pq_node<a,b> *p) const { return p->key; }
  GenPtr&       key(conc_pq_node<a,b> *p) { return p->key; }
  GenPtr&       inf(conc_pq_node<a,b> *p) const { return p->inf; }
  
  conc_pq_node<a,b>* insert(GenPtr p, GenPtr i) ;
  
  void conc(conc_pq_tree<a,b> &pq, int dir = leda::behind);  
  void split_at_item(conc_pq_node<a,b> *p, conc_pq_tree<a,b> &pq1, conc_pq_tree<a,b> &pq2);

  conc_pq_node<a,b>* find_min() const { return minptr; }

  void del_min() { del_item(minptr); }
  void del_item(conc_pq_node<a,b> *p);

  //  void decrease_key(conc_pq_node<a,b> *p, GenPtr k);
  bool decrease_key(conc_pq_node<a,b> *p, GenPtr k);

  // mod. 15/01, added new funtion (needed for multiple search tree approach)
  bool increase_key(conc_pq_node<a,b> *p, GenPtr k);

  int  size()   const { return (root ? root->leaves : 0); }
  bool empty()  const 
  { return ((root == nil) || (cmp(minptr->key, infinity) == 0)); }
  
  virtual void reset();
  virtual void clear();

  void *owner;     // pointer to the owner of the tree

  // GenPtr get_owner(conc_pq_node<a,b>* it) { return (it ? my_tree(it)->owner : nil); }
  virtual void set_owner(GenPtr p) { owner = p; }

  int  height() const { return (root ? root->height : -1); }

  void print() const { print_tree(root, 0); } 

  conc_pq_node<a,b>* first_item() const { return head_leaf; }
  conc_pq_node<a,b>* last_item()  const { return tail_leaf; }
  conc_pq_node<a,b>* next_item(conc_pq_node<a,b> *p) const { return (p ? p->succ : nil); }
  conc_pq_node<a,b>* pred_item(conc_pq_node<a,b> *p) const { return (p ? p->pred : nil); }

  conc_pq_node<a,b>* stl_pred_item(conc_pq_node<a,b> *p) const {  
    error_handler(1,"conc_pq_tree::stl_pred_item not implemented."); 
    return nil; 
  }   
  conc_pq_node<a,b>* stl_next_item(conc_pq_node<a,b> *p) const 
  { return (p ? p->succ : head_leaf); }


  void print_item(conc_pq_node<a,b> *it, ostream &out = cout) const;
  virtual void print(ostream &out, string s, char space = ' ') const;

  friend ostream& operator<<(ostream &out, const conc_pq_tree<a,b> &p) 
  { p.print(out, ""); return out; }

  
  //  LEDA_MEMORY(conc_pq_tree);

};

  


// ----------------------------------------------------------------------
//  Realization of Class `conc_pq_tree'
// ----------------------------------------------------------------------


template <int a, int b>
conc_pq_tree<a,b>::conc_pq_tree() {  
  if (a >= 2 && b >= 2*a-1) {
    head_leaf = tail_leaf = nil;
    root = minptr = nil;
  }
  else 
    error_handler(1, "conc_pq_tree::conc_pq_tree(): invalid arguments for a and b.");
}

template <int a, int b>
conc_pq_tree<a,b>::conc_pq_tree(GenPtr p, GenPtr i) {  
  if (a >= 2 && b >= 2*a-1) {
    copy_key(p);
    copy_inf(i);
    conc_pq_node<a,b>* new_r = new_conc_pq_node(p, i);
    head_leaf = tail_leaf = new_r;
    root = new_r;
    minptr = root;    
  }
  else 
    error_handler(1, "conc_pq_tree::conc_pq_tree(GenPtr p, GenPtr i): invalid arguments for a and b.");
}


template <int a, int b>
conc_pq_node<a,b>* conc_pq_tree<a,b>::init(GenPtr p, GenPtr i) {
  clear();
  copy_key(p);
  copy_inf(i);
  conc_pq_node<a,b>* new_r = new_conc_pq_node(p, i);
  head_leaf = tail_leaf = new_r;
  root = new_r;
  minptr = root;

  return new_r;
}


template <int a, int b>
conc_pq_node<a,b>* conc_pq_tree<a,b>::insert(GenPtr p, GenPtr i) { 
  conc_pq_tree<a,b> cur;
  conc_pq_node<a,b> *x = cur.init(p, i);
  conc(cur);
  return x;
}


template <int a, int b>
conc_pq_tree<a,b>::conc_pq_tree(const conc_pq_tree<a,b>& pq) {
  error_handler(1, "conc_pq_tree::conc_pq_tree(const conc_pq_tree& pq): not implemented yet.");
}


template <int a, int b>
conc_pq_tree<a,b>& conc_pq_tree<a,b>::operator=(const conc_pq_tree<a,b>& pq) {
  error_handler(1, "conc_pq_tree::operator=(...): not implemented yet.");
  return *this;
}


template <int a, int b>
conc_pq_node<a,b>* conc_pq_tree<a,b>::split_node(conc_pq_node<a,b>* p) {
  // in case that p has more than b children, p is split into
  // two nodes r and l having half of the children of p each. 
  // it returns the highest (with max. height) node considered.

  if (p->children <= b) return p;

  conc_pq_node<a,b> *l = p, *f = p->parent;

  // create new node r (to the right of l)
  GenPtr new_key = l->key;
  copy_key(new_key);  
  conc_pq_node<a,b> *r = new_conc_pq_node(new_key, nil);
  r->height = l->height;  

  // determine node cur: all children of f up to 
  // cur (inclusive) stay children of r and the others
  // become children of r. determine the min. key of
  // left half
  conc_pq_node<a,b> *cur = l->child;
  GenPtr minkey = cur->key;

  int cnt = 0;
  while (cnt < p->children / 2) {
    cnt++;
    cur = cur->right;
    if (cmp(minkey, cur->key) > 0) 
      minkey = cur->key;
  }

  // adjust children counter
  r->children = l->children - (cnt+1);
  l->children = cnt+1;
  
  // split sibblings list
  r->child = cur->right;
  cur->right = l->child;
  r->child->left = l->child->left;
  l->child->left->right = r->child;
  l->child->left = cur;

  // (possibly) correct key of l
  if (cmp(l->key, minkey) != 0) {
    copy_key(minkey);
    l->key = minkey;
  }
    
  // set parent information for all (new) children of r
  // and determine the (possibly new) min of all children. 
  // keep track of the number of leaves of all children of r
  cur = r->child;
  minkey = cur->key;
  int leaves_cnt = cur->leaves;
  cur->parent = r;
  cur = cur->right;
  
  while (cur != r->child) {
    leaves_cnt += cur->leaves; 
    cur->parent = r;    
    if (cmp(minkey, cur->key) > 0)
      minkey = cur->key;
    cur = cur->right;
  }

  // compute new number of leaves for l and r
  l->leaves -= leaves_cnt;
  r->leaves  = leaves_cnt;

  // (possibly) correct key of r
  if (cmp(r->key, minkey) != 0) {
    copy_key(minkey);
    r->key = minkey;
  }

  // create new root node (when l's parent is nil)
  // with key = min{l->key, r->key} and only child l
  if (f == nil) {
    minkey = r->key;
    if (cmp(minkey, l->key) > 0) 
      minkey = l->key;
    copy_key(minkey);
    f = new_conc_pq_node(minkey, nil);
    f->child = l;
    f->children = 1;
    f->height = l->height + 1;
    f->leaves = l->leaves + r->leaves;
    l->parent = f;
  }

  // INVARIANT: l is child of f
  // make r a new child of f (to the right of l)
  r->parent = f;
  r->right = l->right;
  r->left  = l;
  l->right->left = r;
  l->right = r;

  f->children += 1;

  // recursively split f
  if (f->children > b) return split_node(f);
  else return f;
}



template <int a, int b>
void conc_pq_tree<a,b>::add_leaves(conc_pq_node<a,b> *p, int leaves) {
  // traverses the path from p up to the root and sets
  // for each node cur on that path:
  // cur.leaves += leaves

  conc_pq_node<a,b>* cur = p;

  while (cur) {
    cur->leaves += leaves;
    cur = cur->parent;
  }
}



template <int a, int b>
conc_pq_node<a,b>* conc_pq_tree<a,b>::conc(conc_pq_node<a,b>* r1, conc_pq_node<a,b>* r2) {
  // concatenates the two trees rooted at r1 and r2 and 
  // returns the root of the concatenated trees.
  // minptr and list of leaves are NOT maintained here

  if (r1 == nil) return r2;
  if (r2 == nil) return r1;

  int h1 = r1->height;
  int h2 = r2->height;

  conc_pq_node<a,b> *cur, *f;

  if (h1 >= h2) {
    // seek rightmost node cur in tree rooted at r1 
    // with height h2 and let f be the father of cur
    cur = r1;
    while (cur->height != h2) cur = cur->child->left;
    f = cur->parent;

    // create new root f with key cur->key in case h1 == h2
    if (f == nil) {      
      GenPtr new_key = cur->key;
      copy_key(new_key);
      f = new_conc_pq_node(new_key, nil);
      f->child = cur;
      f->height = cur->height + 1;	
      f->leaves = cur->leaves;
      f->children = 1;
      cur->parent = f;
    }

    // INVARIANT: f is the parent of cur
    f->child->left = r2;
    r2->right = f->child;
    r2->left  = cur;
    cur->right = r2;
    r2->parent = f;
    
    add_leaves(f, r2->leaves);
    f->children += 1;
    
    if (cmp(f->key, r2->key) > 0)
      decrease_prio(f, r2->key);

    if (f->children > b) f = split_node(f);

    return (f->height > r1->height ? f : r1);
  }
  else { 
    // seek leftmost node cur in tree rooted at r2 
    // with height h1 and let f be the father of cur
    cur = r2;
    // next line mod. 12/01, was before: 
    // while (cur->height != h1) cur = cur->child->left;
    while (cur->height != h1) cur = cur->child;
    f = cur->parent;
    
    // INVARIANT: f is the parent of cur
    r1->left = f->child->left;
    r1->right  = cur;
    r1->parent = f;
    f->child->left->right = r1;
    f->child = r1;
    cur->left = r1;

    add_leaves(f, r1->leaves);
    f->children += 1;

    if (cmp(f->key, r1->key) > 0)
      decrease_prio(f, r1->key);

    if (f->children > b) f = split_node(f);

    return (f->height > r2->height ? f : r2);
  }
}



template <int a, int b>
void conc_pq_tree<a,b>::conc(conc_pq_tree<a,b> &pq, int dir) {

  if (&pq == this) 
    error_handler(1, "void conc_pq_tree::conc(...): argument is identical with object"); 

  conc_pq_tree<a,b> *T1 = (dir == leda::behind ? this : &pq );
  conc_pq_tree<a,b> *T2 = (dir == leda::behind ? &pq  : this);

  // set root
  root = conc(T1->root, T2->root);
  root->tree = this;

  // determine new minptr
  if (T1->minptr == nil)
    minptr = T2->minptr;
  else if (T2->minptr == nil)
    minptr = T1->minptr;
  else {
    conc_pq_node<a,b>* aux = T2->minptr;
    // CAREFUL: minptr might overwrite T2->minptr in the
    // next step, but using aux. variable is save
    minptr = T1->minptr;
    if (cmp(minptr->key, aux->key) > 0)
      minptr = aux;
  }

  // concatenate leave lists (with special cases
  // T1 or T2 empty)

  if (T1->tail_leaf == nil)
    T1->head_leaf = T2->head_leaf;
  else if (T2->tail_leaf == nil)
    T2->tail_leaf = T1->tail_leaf;
  else {
    T1->tail_leaf->succ = T2->head_leaf;
    T2->head_leaf->pred = T1->tail_leaf;
  }

  head_leaf = T1->head_leaf;
  tail_leaf = T2->tail_leaf;

  // destroy object different form *this
  if (T1 != this) {
    T1->head_leaf = T1->tail_leaf = nil;
    T1->root = T1->minptr = nil;
  }
  if (T2 != this) {
    T2->head_leaf = T2->tail_leaf = nil;
    T2->root = T2->minptr = nil;
  }
}



template <int a, int b>
void conc_pq_tree<a,b>::delete_node(conc_pq_node<a,b> *p) {
  // deletes the node p
  
  clear_key(p->key);
  clear_inf(p->inf);  
  std_memory.deallocate_bytes(p, sizeof(conc_pq_node<a,b>));

}


template <int a, int b>
void conc_pq_tree<a,b>::print_tree(conc_pq_node<a,b> *cur, int blancs) const { 

  if (cur == 0) { 
    for(int j = 1; j <= blancs; j++) 
      cout << " ";
    cout << "NIL" << endl;
    return;
  }  
  if (cur->height == 0) { 
    for(int j = 1; j <= blancs; j++) 
      cout << " ";
    if (cmp(cur->key, infinity) == 0) 
      cout << "infty";
    else 
      print_key(cur->key); 
    cout << endl;
  }
  else {         
    conc_pq_node<a,b> *it = cur->child;

    print_tree(it, blancs + 10);
    it = it->right;
    while (it != cur->child) {
      print_tree(it, blancs + 10);
      it = it->right;
    }
    for(int j = 1; j <= blancs; j++) 
      cout << " ";
    if (cmp(cur->key, infinity) == 0) 
      cout << "infty";
    else 
      print_key(cur->key); 
    cout << endl;
  }
}


template <int a, int b>
void conc_pq_tree<a,b>::split_at_item(conc_pq_node<a,b> *p, conc_pq_tree<a,b> &pq1, conc_pq_tree<a,b> &pq2) {
  // splits *this at p into pq1 and pq2. pq1 contains thereafter all elements
  // of *this that preceed p and p itself, pq2 contains all elements strictly after p.
  // NOTICE: relevant for the split operation is the linear precedence relation
  // on the leaves and NOT the key of the elements  
  // case p == nil:         pq1 gets empty and pq2 equals *this
  // case p == last_item(): pq2 gets empty and pq1 equals *this

  if (root == nil) 
    error_handler(1, "void conc_pq_tree::split(...): pq is empty.");
  if (&pq1 == &pq2)
    error_handler(1, "void conc_pq_tree::split(...): identical arguments.");
  
  if (&pq1 != this) pq1.clear();
  if (&pq2 != this) pq2.clear();

  if (p == nil || p == last_item()) {
    // make either pq1 or pq2 a copy of *this
    conc_pq_tree<a,b> *pq = (p == nil ? &pq2 : &pq1);

    // make pq equal to *this
    pq->head_leaf = head_leaf;
    pq->tail_leaf = tail_leaf;
    pq->root = root;
    // mod. 05/01
    pq->root->tree = pq;

    pq->minptr = minptr;
    
    // destroy *this
    if (pq != this) {
      head_leaf = tail_leaf = nil;
      root = minptr = nil;
    }
    return;
  }

  // store pointers to roots of left/right forest in array
  conc_pq_node<a,b> **left_trees  = new conc_pq_node<a,b>*[root->leaves];  
  conc_pq_node<a,b> **right_trees = new conc_pq_node<a,b>*[root->leaves]; 

  int left_cnt  = 0;  
  int right_cnt = 0;

  conc_pq_node<a,b> *cur = p, *f, *l, *r;
  conc_pq_node<a,b> *l_stop, *r_stop;

  while (cur->parent) {
    f = cur->parent;
    l = cur->left;  l_stop = f->child->left; 
    r = cur->right; r_stop = f->child;
     
    // take cur to the left forest, when cur == p (leaf) 
    if (p == cur) left_trees[left_cnt++] = cur;
    
    // collect trees to the left of cur
    while (l != l_stop) {
      left_trees[left_cnt++] = l;
      l = l->left;
    }

    // collect trees to the right of cur
    while (r != r_stop) {
      right_trees[right_cnt++] = r;
      r = r->right;
    }

    if (cur != p) delete_node(cur);
    cur = f;    
  }

  // delete root
  if (cur != p) delete_node(cur);
  
  // destroy *this
  root = nil;

  // concatenate all trees from the left forest
  // -- mod. 12/09/00: for (int i = left_cnt-1 ; i >= 0; i--) 
  int i;
  for (i = 0; i < left_cnt; i++) {
    conc_pq_node<a,b> *cur = left_trees[i];
    cur->parent = nil;
    cur->left = cur->right = cur;
    // -- mod. 12/09/00: pq1.root = conc(pq1.root, cur);
    pq1.root = conc(cur, pq1.root);
  }

  // concatenate all trees from the right forest
  for (i = 0 ; i < right_cnt; i++) {
    conc_pq_node<a,b> *cur = right_trees[i];
    cur->parent = nil;
    cur->left = cur->right = cur;
    pq2.root = conc(pq2.root, cur);
  }

  // determine new min: determine the min of the smaller group.
  // if this is equal to the current minptr (hard luck) the 
  // min of the larger group must be determined as well; otherwise,
  // we're done

  conc_pq_node<a,b> *aux = minptr;
  
  conc_pq_tree<a,b> *small_pq = (pq1.root->leaves > pq2.root->leaves ? &pq2 : &pq1);
  conc_pq_tree<a,b> *large_pq = (pq1.root->leaves > pq2.root->leaves ? &pq1 : &pq2);
  
  small_pq->minptr = locate_min(small_pq->root);

  // mod. 13/01
  // CAREFULL: aux might point to an element different from the minptr of small_pq,
  // _although_ the priority of aux->key and minptr->key are equal. 
  // Eg:  this = (pq1 = 2 1 3 4 | pq2 = 7 8 2 0 0)    (pq1 large, pq2 small)
  //                                            ^aux
  // locate_min in small pq (pq2) yields:     ^
  // Hence, next line does not work
  // -- if (small_pq->minptr == aux) 
  if (cmp(small_pq->minptr->key, aux->key) == 0)
    large_pq->minptr = locate_min(large_pq->root);
  else
    large_pq->minptr = aux;

  // split leaf list
  pq1.head_leaf = head_leaf;
  pq2.tail_leaf = tail_leaf;
  pq1.tail_leaf = p;
  pq2.head_leaf = p->succ;
  pq1.tail_leaf->succ = nil;
  pq2.head_leaf->pred = nil;

  // adjust the tree pointers of the root nodes
  pq1.root->tree = &pq1;
  pq2.root->tree = &pq2;

  // destroy *this
  if ((&pq1 != this) && (&pq2 != this)) {
    minptr    = nil; 
    head_leaf = tail_leaf = nil;
  }

  delete[] left_trees;
  delete[] right_trees;
}




template <int a, int b>
conc_pq_node<a,b>* conc_pq_tree<a,b>::locate_min(conc_pq_node<a,b> *p) const {
  // determines the minimal leaf in tree rooted at p

#define _LOCATE_MIN_MACRO(ktype)\
ktype cur_key, it_key;\
while(cur->child) {\
  it = cur->child;\
  cur_key = LEDA_ACCESS(ktype, cur->key);\
  it_key  = LEDA_ACCESS(ktype, it->key);\
  while (it_key != cur_key) {\
    it = it->right;\
    it_key = LEDA_ACCESS(ktype, it->key);\
  }\
  cur = it;\
}

  conc_pq_node<a,b> *cur = p, *it = nil;

  switch( key_type_id() ) {

  case CHAR_TYPE_ID:   { _LOCATE_MIN_MACRO(char);   } break;
  case INT_TYPE_ID:    { _LOCATE_MIN_MACRO(int);    } break;
  case LONG_TYPE_ID:   { _LOCATE_MIN_MACRO(long);   } break;
  case FLOAT_TYPE_ID:  { _LOCATE_MIN_MACRO(float);  } break;
  case DOUBLE_TYPE_ID: { _LOCATE_MIN_MACRO(double); } break;

  default: {
    while (cur->child) {
      it = cur->child;
      while (cmp(it->key, cur->key) != 0)
	it = it->right;
      cur = it;
    }
  } 
  break;
  }

  return cur;
}



template <int a, int b>
void conc_pq_tree<a,b>::decrease_prio(conc_pq_node<a,b> *p, GenPtr k) {
  // starts at p and follows the path to the root of p.
  // at each node: if current key is larger than k
  // replace with k, and exit otherwise.

  conc_pq_node<a,b> *cur = p;
  
  while (cur && (cmp(cur->key, k) > 0)) {
    copy_key(k);
    cur->key = k;
    cur = cur->parent;
  }         
}



template <int a, int b>
conc_pq_node<a,b>* conc_pq_tree<a,b>::increase_prio(conc_pq_node<a,b> *p, GenPtr k) {
  // starts at p and follows the path to the root of p.
  // at each node: if current key is not the min. of all children
  // replace, otherwise exit.
  // the function returns a conc_pq_node* to the node having minimal 
  // key that has been inspected during the search

#define _INCREASE_PRIO_MACRO(ktype) \
ktype old_key; \
ktype cur_key = LEDA_ACCESS(ktype, cur->key); \
ktype min_key = LEDA_ACCESS(ktype, k); \
while (cur && (cur_key != min_key)) { \
  old_key = cur_key; \
  cur->key = leda_cast(min_key); \
  if (cur->parent && (LEDA_ACCESS(ktype, cur->parent->key) == old_key)) { \
    cur_stop = cur; \
    cur = cur->right; \
    while(cur != cur_stop) { \
      cur_key = LEDA_ACCESS(ktype, cur->key); \
      if (min_key > cur_key) { min = cur; min_key = cur_key; } \
      cur = cur->right; \
    } \
    cur = cur->parent; \
  } \
  if (cur) cur_key = LEDA_ACCESS(ktype, cur->key); \
}

  conc_pq_node<a,b> *cur = p, *cur_stop;
  conc_pq_node<a,b> *min = cur;

  switch( key_type_id() ) {

  case CHAR_TYPE_ID:   { _INCREASE_PRIO_MACRO(char);   } break;
  case INT_TYPE_ID:    { _INCREASE_PRIO_MACRO(int);    } break;
  case LONG_TYPE_ID:   { _INCREASE_PRIO_MACRO(long);   } break;
  case FLOAT_TYPE_ID:  { _INCREASE_PRIO_MACRO(float);  } break;
  case DOUBLE_TYPE_ID: { _INCREASE_PRIO_MACRO(double); } break;

  default: {
    GenPtr old_key;
    GenPtr min_key = k;

    while (cur && (cmp(cur->key, min_key) != 0)) {
      old_key = cur->key;
      copy_key(min_key);
      cur->key = min_key;      
      if (cur->parent && (cmp(cur->parent->key, old_key) == 0)) {
	cur_stop = cur;
	cur = cur->right;	
	while(cur != cur_stop) {
	  if (cmp(min_key, cur->key) > 0) { 
	    min = cur; 
	    min_key = cur->key; 
	  }
	  cur = cur->right;
	}
        cur = cur->parent;
      }      
    }
  }
  break;
  }

  return min;

}


template <int a, int b>
void conc_pq_tree<a,b>::del_item(conc_pq_node<a,b> *p) {
  // we do not really delete an item, but instead set the
  // key to infinity

  if (p == nil) return;

  bool was_min = (p == minptr);

  if (cmp(p->key, infinity) == 0) return;
  else {
    conc_pq_node<a,b> *start = increase_prio(p, infinity);
    if (was_min) minptr = locate_min(start);
  }
}


template <int a, int b>
bool conc_pq_tree<a,b>::decrease_key(conc_pq_node<a,b> *p, GenPtr k) {
  // the key of p is decreased to k. in case k >= p->key 
  // we return false, else true to signalize, if the operation
  // was succesfull, or not

  int r = cmp(k, p->key);

  if (r >= 0) return false;
  else {
    if (cmp(k, minptr->key) < 0)
      minptr = p;
    decrease_prio(p, k);
    return true;
  }
}




template <int a, int b>
bool conc_pq_tree<a,b>::increase_key(conc_pq_node<a,b> *p, GenPtr k) {
  // increases the key of p to k. if p->key >= k false is returned,
  // and true otherwise

  if (p == nil) return false;
  
  bool was_min = (p == minptr);
  
  if (cmp(p->key, k) >= 0) return false;
  else {
    conc_pq_node<a,b> *start = increase_prio(p, k);
    if (was_min) minptr = locate_min(start);
    return true;
  }
}





template <int a, int b>
inline 
void conc_pq_tree<a,b>::traverse_tree(conc_pq_node<a,b>* cur, GenPtr k) {
  // traverses the tree. for each node v: sets the key of v to k 
  // (case k != nil), or deletes v (case k == nil)

  if (cur == nil) return; 

  register conc_pq_node<a,b>* it = cur->child;
  
  while(it && (it != cur->child->left)) {
    traverse_tree(it, k);
    it = it->right;
  }
  if (it) traverse_tree(it, k);
 
  if (k != nil) {
    copy_key(k);
    cur->key = k;
  }
  else {
    clear_key(cur->key);
    clear_inf(cur->inf);
    std_memory.deallocate_bytes(cur, sizeof(conc_pq_node<a,b>));
  }
}


template <int a, int b>
void conc_pq_tree<a,b>::reset() { 
  if (!root) return;
  if (cmp(minptr->key, infinity) == 0) return;
 
  // increase all priorities to infty
  conc_pq_node<a,b> *cur = first_item();
  while (cur) {
    increase_prio(cur, infinity);
    cur = next_item(cur);
  }
}


template <int a, int b>
void conc_pq_tree<a,b>::clear() {
  if (!root) return;

  // delete tree
  traverse_tree(root);
  
  // std_memory.deallocate_list(head_leaf, tail_leaf, sizeof(conc_pq_node<a,b>));
  
  head_leaf = tail_leaf = nil;
  root = minptr = nil;
}




template <int a, int b>
void conc_pq_tree<a,b>::print_item(conc_pq_node<a,b> *it, ostream &out) const {
  if (it == nil) return;
  out << "<";
  print_key(it->key, out);
  out << ", ";
  print_inf(it->inf, out);
  out << ">";
}

template <int a, int b>
void conc_pq_tree<a,b>::print(ostream &out, string s, char space) const {

  out << s;
  conc_pq_node<a,b> *cur = first_item();
  
  while (cur) {
    print_item(cur, out);
    out << space;
    cur = next_item(cur);
  }
  out.flush();
}


template <int a, int b>
inline
conc_pq_node<a,b>* conc_pq_node<a,b>::get_root(conc_pq_node<a,b> *p) {
  conc_pq_node<a,b> *cur = p;  
  if (cur == nil) return nil;    
  while (cur->parent) cur = cur->parent;
  return cur;
}


template <int a, int b>
inline
conc_pq_tree<a,b>* conc_pq_node<a,b>::get_tree(conc_pq_node<a,b> *p) {
  conc_pq_node<a,b> *cur = conc_pq_node<a,b>::get_root(p);
  return (cur == nil) ? nil : cur->tree;
}


template <int a, int b>
inline
GenPtr conc_pq_node<a,b>::get_owner(conc_pq_node<a,b>* it) 
{ return (it ? conc_pq_node<a,b>::get_tree(it)->owner : nil); }



template <int a, int b>
inline
conc_pq_node<a,b>* my_root(conc_pq_node<a,b> *p) {
  conc_pq_node<a,b> *cur = p;
  
  if (cur == nil) return nil;
    
  while (cur->parent) cur = cur->parent;
  
  return cur;
}


template <int a, int b>
inline
conc_pq_tree<a,b>* my_tree(conc_pq_node<a,b> *p) {
  conc_pq_node<a,b> *cur = my_root(p);
  if (cur == nil) return nil;
  else return cur->tree;
}


template <int a, int b>
inline
GenPtr get_owner(conc_pq_node<a,b>* it) 
{ return (it ? conc_pq_node<a,b>::get_tree(it)->owner : nil); }




// ----- #include "concat_pq.h"

/*{\Manpage {concat_pq} {P, I} {Concatenable Priority Queues} {Q}}*/

typedef conc_pq_tree<>::item c_pq_item;

template<class P, class I>
class concat_pq : public /*virtual*/ conc_pq_tree<> {
	typedef conc_pq_tree<> base;

/*{\Mtext 
We implemented a data structure |\Mtype| supporting all needed 
operations of data type {\it concatenable \pq} as introduced in
\secref{sec: concatenable pq}.
The implementation is based on $(a,b)$--trees; we chose $a=2$ and $b=16$.
|concat| and |split_at_item| are essentially realized as discussed
at the end of \secref{sec: concatenable pq}. 
We do not intend to 
go into the implementation details. Instead, the specification of all 
operations needed in the subsequent sections is given.


In \secref{sec: survey} we outlined the idea of using a {\it union--find} data
structure with {\it split} operation to handle the surface graph. 
The method we use in our implementation is different. Since a concatenable \pq\ 
will be assigned to each surface blossom, we decided to extend the functionality 
of |\Mtype| such that it also enables the maintenance of the surface graph.
We use the underlying $(a,b)$--trees to identify a setable object (which will 
be the surface blossom) of a given item (which will correspond to a vertex).
The way this is achieved is as follows. 
Each root of an $(a,b)$--tree stores a pointer to the object representing 
that tree. 
Traversing from an item |it| towards the root, we can identify the 
$(a,b)$--tree object containing the item |it|. 
Moreover, each $(a,b)$--tree object has a generic pointer |owner| (a generic
pointer is of type |void*|) which is setable by the user; see operation 
|set_owner|. Consequently, the |owner| of any item |it| can be 
identified (operation |get_owner|) in time $O(\log n)$, where $n$ denotes the 
number of items in the $(a,b)$--tree.
}*/


  /*{\Mdefinition
    An instance |\Mvar| of the parameterized data type |\Mname| is a collection of items
    (type |c_pq_item|). 
    Every item contains a priority from a linearly ordered type $P$ and an information from 
    an arbitrary type $I$. We use $\Litem{p,i}$ to denote a |c_pq_item| with priority $p$ 
    and information $i$.
    \ignore{|P| is called the priority type and |I| the information type of |\Mvar|.}
    The data structure requires a designated element |infinity| of |P|, with
    $|infinity| \geq p$ for all $p \in P$ and equality holds only if |p=infinity|.
    An item $\Litem{p,i}$ with |p=infinity| is {\it irrelevant} to |\Mvar|. 
    The number of items in |\Mvar| is called the |size| of |\Mvar|. |\Mvar| is {\it empty} 
    when all its items are irrelevant, or when |\Mvar| has size zero. 
    A setable generic pointer |owner| (type |void*|) is associated with |\Mvar|. 
  }*/

#ifdef CONC_PQ_USER_DEF_CMP_FCT
  const leda_cmp_base<P>* cmp_ptr;

  int (*cmp_ptr1)(const P&, const P&);
  
  int cmp(GenPtr x, GenPtr y)  const { 
    if (cmp_ptr1)
      return LEDA_CALL2(*cmp_ptr1, P, x, y);
    else
      return LEDA_CALL2(*cmp_ptr, P, x, y);
  }

  int  ktype_id;
  int  key_type_id() const  { return ktype_id; }  
#else
  int key_type_id() const { return LEDA_TYPE_ID(P); }  
  int cmp(GenPtr x, GenPtr y) const { return LEDA_CALL2(compare, P, x, y); }
#endif

  void clear_key(GenPtr &x) const { LEDA_CLEAR(P, x); }
  void clear_inf(GenPtr &x) const { LEDA_CLEAR(I, x); }
  void copy_key(GenPtr &x)  const { LEDA_COPY(P, x); }
  void copy_inf(GenPtr &x)  const { LEDA_COPY(I, x); }
  void print_key(GenPtr &x, ostream &out = cout)  const { LEDA_PRINT(P, x, out); }
  void print_inf(GenPtr &x, ostream &out = cout)  const { LEDA_PRINT(I, x, out); }


public:

/* {\Mtypes 5} */ 

  typedef c_pq_item item;
  /* {\Mtypemember   the item type.} */

  typedef P prio_type;  
  /* {\Mtypemember   the priority type.} */  

  typedef I inf_type;
  /* {\Mtypemember   the information type.} */ 

// was 5.5
/*{\Mcreation Q 6.2}*/

#ifdef CONC_PQ_USER_DEF_CMP_FCT
  concat_pq(const leda_cmp_base<P>& cmp, P infty) : conc_pq_tree() {
    cmp_ptr  = &cmp; 
    cmp_ptr1 = 0; 
    ktype_id = UNKNOWN_TYPE_ID; 
    base::infinity = leda_cast(infty);
    base::owner = this;
  }
#endif

  concat_pq() : base() {    
#ifdef CONC_PQ_USER_DEF_CMP_FCT
    cmp_ptr1 = compare;
    ktype_id = LEDA_TYPE_ID(P);
#endif
    base::infinity = leda_cast(MAX_VALUE(P));
    base::owner = this;
  }
  /*{\Mcreate   creates an instance |\Mvar| of type |\Mname| based on the linear
    order defined by the global compare function |compare(const P&, const P&)| 
    and initializes it with the empty priority queue. |infinity| is set to the 
    maximum value of type |P|.}*/

  concat_pq(P p, I i) : base(leda_cast(p), leda_cast(i)) {
#ifdef CONC_PQ_USER_DEF_CMP_FCT
    cmp_ptr1 = compare;
    ktype_id = LEDA_TYPE_ID(P); 
#endif
    base::infinity = leda_cast(MAX_VALUE(P));
    base::owner = this;
  }

#ifdef CONC_PQ_USER_DEF_CMP_FCT
  concat_pq(int (*cmp)(const P&, const P&), P infty) : base() {
    cmp_ptr1 = cmp;
    ktype_id = UNKNOWN_TYPE_ID; 
    base::infinity = leda_cast(infty);
    base::owner = this;
  }
  /* {\Mcreate   creates an instance |\Mvar| of type |\Mname| based on the linear order
    defined by the compare function |cmp| and sets the designated element |infinity| 
    to |infty|. |\Mvar| is initialized with the empty priority queue.
    \precond |cmp| must define a linear order on |P| and |infty| satisfies the conditions
    stated above, i.e.~$|cmp|(|infty|, p) \geq 0$ for all $p \in P$.} */
#endif

  ~concat_pq() { base::clear(); }


// was 1.8 3.7
/*{\Moperations 1.8 4.4}*/   

  P& infinity() { return LEDA_ACCESS(P, base::infinity); }
  /* {\Mop   returns a reference to the value of |infinity|.} */

  c_pq_item init(P p, I i) {
    return base::init(leda_cast(p), leda_cast(i));
  }
  /*{\Mop   initializes |\Mvar| to the priority queue containing 
    only the item $\Litem{p, i}$ and returns that item.}*/

  virtual const P& prio(c_pq_item it) const { 
    return LEDA_CONST_ACCESS(P, base::key(it)); 
  }
  /*{\Mop   returns the priority of item $it$.
    \precond $it$ is an item in |\Mvar|.}*/

  virtual I& inf(c_pq_item it) {
    return LEDA_ACCESS(I, base::inf(it)); 
  }

  virtual const I& inf(c_pq_item it) const { 
    return LEDA_CONST_ACCESS(I, base::inf(it)); 
  }
  /*{\Mop   returns the information of item $it$.
    \precond $it$ is an item in |\Mvar|.}*/

  virtual c_pq_item insert(P p, I i) {
    return base::insert(leda_cast(p), leda_cast(i));
  }
  /* {\Mop   adds a new item $\Litem{p, i}$ to |\Mvar| and returns it. All items in
    |\Mvar| precede $\Litem{p, i}$.} */


  void concat(concat_pq<P,I> &pq, int dir = leda::behind) {
    base::conc(pq, dir);
  }
  /*{\Mop   concatenates |\Mvar| with |pq|. The items in |\Mvar| precede (succeed)   
    the items of |pq|, when $|dir|=|behind|$ ($|dir|=|before|$). |pq| is 
    made empty, i.e.~contains no items thereafter.}*/

  void split_at_item(c_pq_item it, concat_pq<P,I> &pq1, concat_pq<P,I> &pq2) { 
    base::split_at_item(it, pq1, pq2); 
  }
  /*{\Mopl  splits |\Mvar| at item |it| into |pq1| and |pq2| such that |it| is 
    the last item of |pq1|. 
    In case $|it|=|nil|$, |pq2| becomes |\Mvar| and |pq1| becomes empty.
    The instance |\Mvar| is empty thereafter, unless it is given as one
    of the arguments.}*/

  virtual c_pq_item find_min() const { return base::find_min(); }
  /*{\Mop   returns an item with minimal priority (|nil| if |\Mvar| is empty).}*/

  virtual P del_min() { 
    P x = prio(find_min()); 
    base::del_min(); 
    return x; 
  }
  /*{\Mop   makes the item $|it|=|\Mvar.find_min|()$ irrelevant to |\Mvar| by
    setting its priority to |infinity|. The former priority is returned.}*/
     
  virtual void del_item(c_pq_item it) { base::del_item(it); }
  /*{\Mopl   makes the item $it$ irrelevant to |\Mvar|.
    \precond |it| is an item in |\Mvar|.}*/

  virtual bool decrease_p(c_pq_item it, const P& x) { 
    return base::decrease_key(it, leda_cast(x)); 
  }
  /*{\Mopl   makes $x$ the new priority of item $it$. The function returns |true| iff
    the operation was successful, i.e.~|\Mvar.prio(it)| was larger than $x$.}*/

  virtual bool increase_p(c_pq_item it, const P& x) { 
    return base::increase_key(it, leda_cast(x)); 
  }
  /*{\Mopl   makes $x$ the new priority of item $it$. The function returns |true| iff
    the operation was successful, i.e.~|\Mvar.prio(it)| was smaller than $x$.}*/

  virtual const I& operator[](c_pq_item it) const
  { return LEDA_CONST_ACCESS(I, base::inf(it)); }

  
  virtual I& operator[](c_pq_item it) 
  { return LEDA_ACCESS(I, base::inf(it)); }
  /* {\Marrop   returns a reference to the information of item $it$.
    \precond $it$ is an item in |\Mvar|.} */


  int  size() const { return base::size(); }
  /*{\Mop   returns the size of |\Mvar|.}*/


  bool empty() const { return base::empty(); }
  /*{\Mop   returns |true|, if |\Mvar| is empty, and |false| otherwise.}*/


  void reset() { base::reset(); }
  /*{\Mop   makes |\Mvar| the empty priority queue by setting all priorities to |infinity|.}*/


  void clear() { base::clear(); }
  /*{\Mop   makes |\Mvar| the empty priority queue by deleting all items. }*/


  void set_owner(GenPtr pt) { base::set_owner(pt); }
  /*{\Mop   sets |owner| of |\Mvar| to the object pointed to by the 
            generic pointer |pt| (type |void*|).}*/


/*{\Mtext \mansection{Friend Functions}}*/

/*
  friend GenPtr get_owner(c_pq_item it);
*/
  /*{\Mfunc returns the generic pointer |owner| of the instance containing item |it|.}*/


/*{\Mtext \mansection{Iteration}

{\bf forall\_items}($it, Q$)       
$\{$ ``the items of $Q$ are successively assigned to $it$'' $\}$

{\bf forall}($i, Q$)       
$\{$ ``the information parts of the items of $Q$ are successively assigned to $i$'' $\}$ 
}*/


};

c_pq_item my_root(c_pq_item it);
/* {\Mfunc   returns the root of item |it|.} */


template<class P, class I> 
inline c_pq_item create_concat_pq(P p, I i) {
  concat_pq<P, I> *pq = new concat_pq<P, I>;
  return pq->init(p, i);
}

template<class P, class I> 
inline c_pq_item create_concat_pq(P p, I i, concat_pq<P, I>* &pq) {
  pq = new concat_pq<P, I>;
  return pq->init(p, i);
}

template<class P, class I> 
inline void my_concat_pq(c_pq_item it, concat_pq<P, I>* &pq) {
  pq = (concat_pq<P, I>*)(my_tree(it)->owner);
}
/* {\Mfunc   returns a pointer to the instance of type |\Mname| that contains 
  item |it| in |pq|.} */



/*{\Mimplementation
  All access operations take time O(1). 
  |concat| and |split_at_item| take time $O(\log n)$, where $n$ is the (maximum) number of 
  elements in the priority queue(s). Operations |clear| and |reset| take time $O(n)$.
  All other operations take time (at most) $O(\log n)$.
}*/         


// ----- #include "greedy.t"


#if defined(_CHECK) && !defined(_LEDA_DEBUG)
#undef _CHECK
#endif

template<class NT>
__temp_func_inline
int greedy_matching(const graph &G, const edge_array<NT> &w, 
                    node_array<NT> &pot, node_array<node> &mate, 
                    bool perfect) {
#ifdef _INFO
  cout << "\t\tCREATING greedy matching..." << flush;
  float t = used_time();
#endif

  
  edge e;
  node u, v;
  pot.init(G, -MAX_VALUE(NT));

  forall_nodes(u, G)
    if (degree(u) == 0) pot[u] = 0;

  forall_edges(e, G) {
    u = source(e);
    v = target(e);
    pot[u] = leda_max(pot[u], (w[e]/2));
    pot[v] = leda_max(pot[v], (w[e]/2));
  }
  
  int free = G.number_of_nodes();
  mate.init(G, nil);

  forall_edges(e, G) {
    u = source(e);
    v = target(e);
    if ((pot[u] + pot[v] == w[e]) &&
         (mate[u] == nil) && (mate[v] == nil)) {
      mate[v] = u;
      mate[u] = v;
  #ifdef MW_MATCHING_DEBUG
      if (debug) cout << "(" << index(u) << ", " << index(v) << ")" << endl;
  #endif
      free -= 2;
    }    
  }
  
  if (perfect) {
    forall_nodes(u, G) {
      if (!mate[u]) {
        NT slack = MAX_VALUE(NT);
        forall_inout_edges(e, u) {
          v = opposite(u, e);
          slack = leda_min(pot[u] + pot[v] - w[e], slack);
        }
        pot[u] -= slack;
      }
    }
  }
#ifdef _CHECK
  cout << "\t\tCHECK FEASIBILITY..." << flush;
  check_feasibility(G, w, pot, mate, perfect);
  cout << "OK!" << endl;
#endif
#ifdef _INFO
  printf("ready! %d free vertices. TIME: %3.2f sec.\n", free, used_time(t));
#endif

  return free;
}





extern bool debug;

void alternate_cycle(node u, node_array<node> &mate, 
                     node_array<node> &pred) {

  node cur1 = pred[u];  
  while (cur1 != u) {
    mate[pred[cur1]] = cur1;
    mate[cur1] = pred[cur1];
    node h = pred[cur1];
    pred[cur1] = nil;
    cur1 = h;
    h = pred[cur1];
    pred[cur1] = nil;
    cur1 = h;
  }
  pred[u] = nil; 
}

void alternate_path(node u, node_array<int> &label,
                    node_array<node> &mate, node_array<node> &pred) {

  node cur = u;
  node pre = nil, nxt;
  
  while (cur) {
    if (label[cur] == even) {
      nxt = mate[cur];
      mate[cur] = pre;
      cur = nxt;      
    }
    else {
      pre = cur;
      mate[cur] = pred[cur];
      nxt = pred[cur];
      pred[cur] = nil;
      cur = nxt;      
    }  
  }
}

template<class NT>
__temp_func_inline
void destroy_tree(node_slist &T, node_array<int> &label, node_array<NT> &pot, 
                  node_array<node> &mate, node_array<node> &pred, NT Delta) {

  node v;
  while (!T.empty()) {
    v = T.pop();
    if (label[v] == even) pot[v] -= Delta;
    else {
      pot[v] += Delta;
      if (mate[v]) pred[v] = nil;  // only for vertices not on cycle
    }      
    label[v] = unlabeled;
  }
}

void seek_lca(node u, node v, node &lca, 
              node_array<node> &mate, node_array<node> &pred,
              node_array<double> &P1, node_array<double> &P2, 
              double &lock) {

  node cur1 = u, cur2 = v;
  P1[cur1] = P2[cur2] = ++lock;

  while ((P1[cur2] != lock) && (P2[cur1] != lock) && 
         (mate[cur1] || mate[cur2])) {
    
    if (mate[cur1]) {      
      cur1 = pred[mate[cur1]];
      P1[cur1] = lock;
    }
    if (mate[cur2]) {
      cur2 = pred[mate[cur2]];
      P2[cur2] = lock;
    }
  }
  if (P1[cur2] == lock)  // cur2 is lca
    lca = cur2;  
  else if (P1[cur1] == lock)  // cur1 is lca
    lca = cur1;
  else lca = nil;
}

void construct_cycle(node u, node v, node lca, 
                     node_array<node> &mate, node_array<node> &pred) {

  node cur1 = u, cur2 = v; 
  while (cur1 != lca) {
    // set pred data to reversed tree path; delete mate entries
    node h = mate[cur1];
    mate[cur1] = nil;
    cur1 = pred[h];
    pred[h] = mate[h];
    mate[h] = nil;
    pred[cur1] = h;
  }
  while (cur2 != lca) {
    // set pred data to tree path; delete mate entries
    node h = mate[cur2];
    pred[cur2] = mate[cur2];
    mate[cur2] = nil;
    mate[h] = nil;
    cur2 = pred[h];
  }  
  pred[u] = v;
#ifdef _CHECK
  if (debug) cout << "check cycle..." << index(cur1) << "  " << flush; 
  // check correctness of odd cycle
  cur1 = pred[lca];
  int cnt = 1;
  while (cur1 != lca) {
    cur1 = pred[cur1];
    assert(mate[cur1] == nil);
    cnt++;
  }
  assert(cnt % 2);
#endif
}

template<class NT> 
__temp_func_inline
int jump_start(const graph &G, const edge_array<NT> &w, 
               node_array<NT> &pot, node_array<node> &mate, 
               bool perfect) {
#ifdef _INFO
  cout << "\tSOLVING half integral matching problem..." << endl;
  float t = used_time();
#endif
  
  
  edge e; 
  node u, v, r;

  node_array<int>  label(G);
  node_array<node> pred(G, nil);

  NT               delta1;
  NT               delta2a;
  node_pq<NT>      delta2b(G); 

  node             resp_d1  = 0;
  edge             resp_d2a = 0;
  node_array<edge> resp_d2b(G);

  NT               Delta = 0;

  slist<edge>        tight;   
  slist<node>        Q;       
  node_slist         T(G);

  double             lock = 0;
  node_array<double> P1(G, 0);
  node_array<double> P2(G, 0); 
  
  int free = greedy_matching(G, w, pot, mate, perfect);
  if (free == 0) return free;
  forall_nodes(u, G) 
    label[u] = (mate[u] ? unlabeled : even);

  forall_nodes(r, G) {
    if (mate[r] || pred[r]) continue;
#ifdef MW_MATCHING_DEBUG
    if (debug) cout << "Grow search tree from " << index(r) << endl;
#endif

    
    delta1 = delta2a = MAX_VALUE(NT);
    delta2b.clear(); 
    Q.clear(); tight.clear(); 

    pot[r] += Delta;
    T.append(r); Q.append(r);

    bool terminate = false;
    while (!terminate) {

      
      while (!Q.empty()) {
        u = Q.pop();    
        NT pot_u = pot[u] - Delta;

        if (!perfect) {
          
          if (pot_u < leda_min(delta1, delta2a) - Delta) {
            delta1 = pot_u + Delta;
            resp_d1 = u;
            if (delta1 == Delta) break;
          }
        }
        forall_inout_edges(e, u) {
          v = opposite(u, e);
          if (label[v] == odd) continue;
          NT pot_v = pot[v] - (((label[v] == even) && T.member(v)) ? Delta : 0);
          NT pi = pot_u + pot_v - w[e];
                  
          if (pi == 0) { 
            
            if ((label[v] == unlabeled) && mate[v]) tight.append(e);
            else {
              tight.clear(); Q.clear();
              tight.append(e);
              break;
            }
          }
          else {  
            
            #if !defined(_NO_PRUNING)
            if (label[v] == even && T.member(v)) {
              if (pi/2 + Delta >= leda_min(delta1, delta2a)) continue;
            }
            else if (pi + Delta >= leda_min(delta1, delta2a)) continue;
            #endif
            if ((label[v] == unlabeled) && mate[v]) {
              
              if (delta2b.member(v)) {
                if (pi < delta2b.prio(v) - Delta) {
                  delta2b.decrease_p(v, pi + Delta);
                  resp_d2b[v] = e;
                }
              }
              else {
                delta2b.insert(v, pi + Delta);
                resp_d2b[v] = e;
              }
            }
            else {
              
              if ((label[v] == even) && T.member(v)) pi /= 2;

              if (pi < delta2a - Delta) {
                delta2a = pi + Delta;
                resp_d2a = e;
              }
            }
          }   
        }    
      }

      if (delta1 == Delta) { 
        
        alternate_path(resp_d1, label, mate, pred);
        destroy_tree(T, label, pot, mate, pred, Delta);
        label[resp_d1] = even;
        terminate = true; 
      }
      else if (!tight.empty()) { 
        
        while (!tight.empty()) {
          e = tight.pop();
          u = (T.member(source(e)) ? source(e) : target(e));
          v = opposite(u, e);

          if (label[v] == odd || label[u] == odd) continue;

          if (label[v] == unlabeled) {
            if (mate[v]) {  
              
              label[v] = odd;
              pred[v] = u;
              pot[v] -= Delta;
              T.append(v);
              delta2b.del(v);
              resp_d2b[v] = nil;

              node m = mate[v]; 
              label[m] = even;
              pot[m] += Delta;
              T.append(m);
              Q.append(m);
              delta2b.del(m);
              resp_d2b[m] = nil;
            }
            else {  // v on half-valued cycle
              
              alternate_cycle(v, mate, pred);
              alternate_path(u, label, mate, pred);
              mate[u] = v;
              mate[v] = u;
              destroy_tree(T, label, pot, mate, pred, Delta);
              free--;
              terminate = true;
              break;
            }
          }
          else {  // label[v] == even
            if (T.member(v)) { 
              
              node lca;
              seek_lca(u, v, lca, mate, pred, P1, P2, lock);
              alternate_path(lca, label, mate, pred);
              construct_cycle(u, v, lca, mate, pred);
              destroy_tree(T, label, pot, mate, pred, Delta);
              free--;
              terminate = true; 
              break;
            }
            else { 
              
              alternate_path(u, label, mate, pred);
              mate[u] = v;
              mate[v] = u;
              label[v] = unlabeled;
              destroy_tree(T, label, pot, mate, pred, Delta);
              free -= 2;
              terminate = true; 
              break;
            }
          }           
        } 
      } // eof if (!tight.empty())
      else {
        
        NT cand2b = (delta2b.empty() ? \
                     MAX_VALUE(NT) : delta2b.prio(delta2b.find_min()));

        NT delta = leda_min(delta1, leda_min(delta2a, cand2b));

        if (delta == MAX_VALUE(NT) && perfect) {
          mate.init(G, nil);
          return 0; 
        }
        #ifdef MW_MATCHING_DEBUG  
        if (debug) {
          cout << "\tPERFORMED DUAL ADJUSTMENT by " << delta-Delta << endl;
          cout << "\tVALUE OF DELTA: " << delta << endl;
        }
        #endif
        Delta = delta;  // corresponds to Delta += (delta - Delta)
        
        if (delta1 == Delta)
          continue;
        else if (delta2a == Delta) {
          tight.append(resp_d2a);
          resp_d2a = nil;
        }
        else {
          while (!delta2b.empty() && 
                 (delta2b.prio(delta2b.find_min()) == Delta)) {
            u = delta2b.del_min();
            tight.append(resp_d2b[u]);
            resp_d2b[u] = nil;
          }
        }
      }
    }
  }
  
  forall_nodes(u, G)
    if (pred[u]) {
      alternate_cycle(u, mate, pred);
      free++;
    } 
#ifdef MW_MATCHING_DEBUG
  cout << "matching after jump start: " << endl;
  node vv;
  forall_nodes(vv, G) 
	if (mate[vv] && index(vv) < index(mate[vv]))
      cout << index(vv) << "<->" << index(mate[vv]) << " ";
  cout << endl;
  cout << "node potentials (no blossoms): " << endl;
  forall_nodes(vv, G) cout << index(vv) << ":" << pot[vv] << " ";
  cout << endl;
#endif
#ifdef _CHECK
  cout << "\t\tCHECK FEASIBILITY..." << flush;
  check_feasibility(G, w, pot, mate, perfect);
  cout << "OK!" << endl;
#endif
#ifdef _INFO
  printf("\tready! %d free vertices. TIME: %3.2f sec.\n", free, used_time(t));
#endif

  return free;
}

template<class NT> 
__temp_func_inline
list<edge> fractional_matching(const graph &G, 
                               const edge_array<NT> &w, 
                               node_array<NT> &pot,
                               bool perfect) {

  node_array<node> mate;

  jump_start(G, w, pot, mate, perfect);

  list<edge> M;

  node_array<int> b(G, -1);
  array<two_tuple<NT, int> > BT;
  int k = 0;

  edge e; node u, v;
  forall_edges(e, G) {
    u = source(e);
    v = target(e);
  
    if (mate[u] && (mate[u] == v) &&
        mate[v] && (mate[v] == u))
    M.push(e);
  }

#ifdef _CHECK
  if (!perfect) 
    CHECK_MAX_WEIGHT_MATCHING(G, w, M, pot, BT, b);
  else     
    CHECK_MAX_WEIGHT_PERFECT_MATCHING(G, w, M, pot, BT, b);
#endif 

  return M;
}


/// SST APPROACH ////////////////////////////////////////////////////////////////////////////

#if defined(_SST_APPROACH)



#ifdef _STAT
int   adj_c, 
      alternate_c, grow_c, shrink_c, expand_c, augment_c, destroy_c,
      scan_c;             
float alternate_t, grow_t, shrink_t, expand_t, augment_t, destroy_t, 
      scan_t;
float heur_t, init_t, alg_t, extract_t; 
#endif


template<class NT> class vertex; 
template<class NT> class blossom;


template<class NT> 
inline blossom<NT>* blossom_of(c_pq_item it, NT) {
  return (it ? (blossom<NT>*)(get_owner(it)) : nil);
}

template<class NT> 
inline c_pq_item new_blossom(NT d, node b, blossom<NT>* &B) {
  B = new blossom<NT>(b);
  vertex<NT> *v = new vertex<NT>(d, b);
  return B->init(MAX_VALUE(NT), v);
} 

template<class NT> 
class blossom : public /*virtual*/ concat_pq<NT, vertex<NT>*> { /* SST */

  struct info {
    list<node>         shrink_path;
    list<blossom<NT>*> subblossom_p; 
    c_pq_item          split_item;

	info() : split_item(nil) {}
	LEDA_MEMORY(info)
  };
  info* _info;
  info& get_info() { if (!_info) _info = new info; return *_info; }

public:
  
  LABEL label;
  NT    pot;
  NT    offset;

  node base, mate;
  node disc, pred;

  list<node>&         shrink_path() { return get_info().shrink_path; }
  list<blossom<NT>*>& subblossom_p() { return get_info().subblossom_p;}
  c_pq_item&          split_item() { return get_info().split_item; }

  bool has_subblossoms() const { return _info != nil && !_info->subblossom_p.empty(); }

  pq_item item_in_pq;

  list_item item_in_T;
  list_item item_in_O;

  double marker1, marker2;
  
  const NT min_prio() const 
  { return (find_min() ? prio(find_min()) : MAX_VALUE(NT)); }

  NT   pot_of  (c_pq_item it) const { return inf(it)->pot;      } 
  node node_of (c_pq_item it) const { return inf(it)->my_node;  }
  node best_adj(c_pq_item it) const { return inf(it)->best_adj; }

  bool trivial() const { return size() == 1; }

  void print_vertices() const {
    vertex<NT> *Cur;
    forall(Cur, *this) cout << index(Cur->my_node) << "  ";
  }

  blossom(node ba = nil) : concat_pq<NT, vertex<NT>*>() {

    set_owner(leda_cast(this));
    label = even;
    pot = offset = 0; 
    base = ba; mate = nil;
    disc = pred = nil;
    marker1 = marker2 = 0;
    item_in_T  = item_in_O = nil; 
    item_in_pq = nil;

	_info = nil;
  }

  ~blossom() { kill(); }

  void kill() {
    clear();
	delete _info; _info = nil;
  }

  void destroy() {
    c_pq_item it;
    forall_items(it, *this)
      delete this->inf(it);
    this->kill();
    delete this;
  }

  void status_change(LABEL l, NT Delta, list<blossom<NT>*> &T, node_slist &Q) {

    if (l == unlabeled) {
      
      assert((label != l) && item_in_T);
      offset += (label == odd ? Delta : -Delta);
      T.del(item_in_T);
      item_in_T = nil;
    }
    else if (l == odd) {
      
      assert((label != l) && !item_in_T);
      offset -= Delta;
      item_in_T = T.append(this);    
    }
    else if (l == even) {
      
      assert((label != l) || !item_in_T);
      if (label == odd) offset += 2*Delta;
      else {  // non-tree blossom
        offset += Delta;
        item_in_T = T.append(this);
      }  
      
      c_pq_item it;
      forall_items(it, *this) {
        inf(it)->pot += offset;
        Q.append(node_of(it));
      }
      if (!trivial()) pot -= 2*offset;
      offset = 0;
    }
    label = l;
  }

  bool improve_connection(c_pq_item it, NT x, node u) {

    if (!it) return false;
    NT old_min = min_prio();
    if (decrease_p(it, x)) inf(it)->best_adj = u;
    return old_min != min_prio();    
  }

  bool delete_connection(c_pq_item it) {

    if (!it) return false;
    NT old_min = min_prio();
    del_item(it);
    inf(it)->best_adj = nil;
    return old_min != min_prio();
  }

  void append_subblossom(blossom<NT> *CUR, NT Delta,
                         list<blossom<NT>*> &T, node_slist &Q) {

    if (CUR->label == odd) 
      CUR->status_change(even, Delta, T, Q);

    if (!CUR->trivial())
      CUR->pot += -2*CUR->offset + 2*Delta;

    T.del(CUR->item_in_T);
    CUR->item_in_T = nil;

    concat(*CUR);
    CUR->split_item() = last_item();
    subblossom_p().append(CUR);   
  }

  void expand(NT Delta) {

    blossom<NT> *CUR;
    forall(CUR, subblossom_p()) {
      split_at_item(CUR->split_item(), *CUR, *this);
      CUR->offset = offset;
      CUR->label = label;

      if (!CUR->trivial() && label == odd)
        CUR->pot += 2*offset + 2*Delta;
      else if (!CUR->trivial()) {
        assert(!CUR->item_in_T);
        assert(CUR->label == even || CUR->label == unlabeled);
        CUR->pot += 2*offset;
      }
    }
  }

  int restore_matching(blossom<NT> *BASE, blossom<NT> *DISC) {
    
    while (subblossom_p().tail() != BASE) {
      subblossom_p().append(subblossom_p().pop());
      shrink_path().append(shrink_path().pop());
      shrink_path().append(shrink_path().pop());
    }
    
    BASE->mate = mate;
    BASE->base = base;

    node ba, m;
    int dist = 0, pos = 1;
    list_item p_it   = shrink_path().first();
    list_item sub_it = subblossom_p().first();
    blossom<NT> *CUR = subblossom_p().inf(sub_it), *ADJ;

    while (CUR != BASE) {
      if (CUR == DISC) dist = pos;
      sub_it = subblossom_p().succ(sub_it); pos++;
      ADJ    = subblossom_p().inf(sub_it);
      if (ADJ == DISC) dist = pos; 

      p_it = shrink_path().succ(p_it); 
      p_it = shrink_path().succ(p_it); ba = shrink_path().inf(p_it);
      p_it = shrink_path().succ(p_it); m = shrink_path().inf(p_it);

      CUR->base = ba; CUR->mate = m;
      ADJ->base = m; ADJ->mate = ba;
         
      sub_it = subblossom_p().succ(sub_it); pos++;
      CUR    = subblossom_p().inf(sub_it);
      p_it   = shrink_path().succ(p_it);
    }
    if (CUR == DISC) dist = pos; 
    return dist;
  }

  LEDA_MEMORY(blossom<NT>);
};

template<class NT> class vertex { /* SST */

public:

  NT   pot;  
  node my_node;  
  node best_adj;

  vertex(NT d, node u) {
    pot = d;
    my_node = u;
    best_adj = nil;
  }

  LEDA_MEMORY(vertex<NT>);
};

#define _BLOSSOM_OF(this_node) \
(this_node ? blossom_of(item_of[this_node],NT(0)) : nil)

template<class NT>
inline NT compute_potential(blossom<NT> *CUR, NT Delta, c_pq_item it) {

  int a = (it == nil ? -2 : 1);
  int sigma = 0;
  if (CUR->item_in_T) sigma = (CUR->label == even ? -1 : 1);
  NT stored = (it == nil ? CUR->pot : CUR->pot_of(it));

  return stored + a * CUR->offset + a * sigma * Delta;
}

// sn: still some compilers do not support default arguments for 
//     template functions

template<class NT>
inline NT compute_potential(blossom<NT> *CUR, NT Delta) {
    return compute_potential(CUR,Delta, (c_pq_item)nil);
}


/* SST */
template<class NT> 
__temp_func_inline
void alternate_path(blossom<NT>* RESP, node_array<c_pq_item> &item_of) {

  if (!RESP) return;
  blossom<NT> *CUR = RESP;
  node pred = RESP->base, disc = nil, mate;

  while (CUR) {
    if (CUR->label == even) {
      mate = CUR->mate;
      CUR->mate = disc;
      CUR->base = pred;
      CUR = _BLOSSOM_OF(mate);
    }
    else {  // CUR->label == odd
      pred = CUR->pred;
      disc = CUR->disc;
      CUR->mate = pred;
      CUR->base = disc;
      CUR = _BLOSSOM_OF(pred);    
    }
  }
}

template <class NT> 
__temp_func_inline
int restore_matching(blossom<NT>* RESP, blossom<NT>* BASE, blossom<NT>* DISC,
                     list<blossom<NT>*> &subblossom_p, list<node> &shrink_path) {

  BASE->mate = RESP->mate;
  BASE->base = RESP->base;

  while (subblossom_p.tail() != BASE) {
    subblossom_p.append(subblossom_p.pop());
    shrink_path.append(shrink_path.pop());
    shrink_path.append(shrink_path.pop());
  }

  int dist, pos = 1;
  node base, mate;

  list_item p_it   = shrink_path.first();
  list_item sub_it = subblossom_p.first();
  blossom<NT>* CUR = subblossom_p.inf(sub_it), *ADJ;

  while (CUR != BASE) {

    if (CUR == DISC) dist = pos;

    pos++;
    sub_it = subblossom_p.succ(sub_it);
    ADJ    = subblossom_p.inf(sub_it);

    if (ADJ == DISC) dist = pos; 
  
    p_it = shrink_path.succ(p_it);
    p_it = shrink_path.succ(p_it); base = shrink_path.inf(p_it);
    p_it = shrink_path.succ(p_it); mate = shrink_path.inf(p_it);

    CUR->base = base; CUR->mate = mate;
    ADJ->base = mate; ADJ->mate = base;
    
    pos++;
    sub_it = subblossom_p.succ(sub_it);
    CUR    = subblossom_p.inf(sub_it);
    p_it   = shrink_path.succ(p_it);
  }
  if (CUR == DISC) dist = pos;
  return dist;
}

template<class NT> 
__temp_func_inline
void unpack_blossom(blossom<NT> *RESP, const node_array<c_pq_item> &item_of,
                    node_array<NT> &pot, node_array<int> &b, 
                    array<two_tuple<NT, int> > &BT,
                    int &k, int parent, NT Delta) {

  if (RESP->trivial()) {
    
    node cur = RESP->node_of(RESP->first_item());
    if (b[cur] != -1) return;
    pot[cur] = compute_potential(RESP, Delta, item_of[cur]);
    b[cur] = parent;      
    #ifdef MW_MATCHING_DEBUG
      if (debug) cout << "trivial blossom " << index(cur) << ": dv:" << pot[cur] 
                      << " immediate super blossom: " << b[cur] << endl;
    #endif
  }
  else {
    
    if (k > BT.high()) BT.resize(2*k+1);
    BT[k].first()  = compute_potential(RESP, Delta);
    BT[k].second() = parent;
    int idx = k++;
    #ifdef MW_MATCHING_DEBUG
      if (debug) {
        cout << "non-trivial blossom "; RESP->print_vertices();
        cout << ": dv: " << BT[idx].first() << ", index: " << idx << " and parent blossom " 
             << BT[idx].second() << endl;
      }
    #endif

    RESP->expand(Delta);
    blossom<NT> *BASE = _BLOSSOM_OF(RESP->base);
    blossom<NT> *DISC = nil;
    RESP->restore_matching(BASE, DISC);

    blossom<NT>* CUR;
    forall(CUR, RESP->subblossom_p()) 
      unpack_blossom(CUR, item_of, pot, b, BT, k, idx, Delta);

    // mod. 12/09/00
    RESP->destroy();
  }
}

template<class NT>
__temp_func_inline
void print_pqs(const NT delta1, node resp_d1,
               const NT delta2a, edge resp_d2a, 
               const p_queue<NT, blossom<NT>*> &delta2b,
               const p_queue<NT, edge> &delta3,
               const p_queue<NT, blossom<NT>*> &delta4,
               const NT Delta, bool perfect) {

  pq_item it;

  if (!perfect) 
    cout << "delta1: " << delta1-Delta << " (" << index(resp_d1) << ")" << endl;
  cout << "delta2a: ";
  if (delta2a != MAX_VALUE(NT)) 
    cout << delta2a-Delta << " (" << index(source(resp_d2a)) << ", " 
         << index(target(resp_d2a)) << ")" << endl;
  else cout << delta2a << endl;

  cout << "delta2b: " << endl;
  forall_items(it, delta2b) {
    cout << delta2b.prio(it) - Delta << "\t"; 
    delta2b.inf(it)->print_vertices(); cout << endl;
  }

 cout << "delta3: " << endl;
  forall_items(it, delta3)
    cout << delta3.prio(it) - Delta << "\t"
         << "(" << index(source(delta3.inf(it))) << ", " 
         << index(target(delta3.inf(it))) << ")" << endl;

  cout << "delta4: " << endl;
  forall_items(it, delta4) {
    cout << delta4.prio(it) - Delta << "\t";
    delta4.inf(it)->print_vertices(); cout << endl;
  }
}

template<class NT> 
__temp_func_inline
list<edge> MWM_SST(const graph &G, const edge_array<NT> &w, 
                   node_array<NT> &pot, array<two_tuple<NT, int> > &BT, 
                   node_array<int> &b, int heur/* =1*/, bool perfect/* =false*/)
{
#ifdef _STAT
  adj_c = alternate_c = grow_c = shrink_c = expand_c = augment_c = scan_c = destroy_c = 0;
  heur_t = init_t = alg_t = extract_t = alternate_t = grow_t = shrink_t = \
expand_t = augment_t = scan_t = destroy_t = 0;
  init_t = used_time();
#endif

  
  NT                        delta1; 
  NT                        delta2a; 
  p_queue<NT, blossom<NT>*> delta2b;
  p_queue<NT, edge>         delta3;         
  p_queue<NT, blossom<NT>*> delta4; 

  node resp_d1;
  edge resp_d2a;

  NT Delta = 0;

  node_slist Q(G);

  list<blossom<NT>*> T;
  list<blossom<NT>*> O;

  node_array<c_pq_item> item_of(G);

  edge        e;
  node        resp, opst, cur, adj, u, v, r;
  blossom<NT> *RESP, *OPST, *CUR, *ADJ, *R;

  list<edge>  M;

  const NT INFTY = MAX_VALUE(NT);

  double lock = 0;
  
  int free = G.number_of_nodes();
  node_array<node> mate(G, nil);

  #ifdef _STAT
  heur_t = used_time();
  #endif
  switch(heur) {
    case 0:  { 
               forall_nodes(u, G) {
                 if (degree(u) == 0) { pot[u] = 0; continue; }
                 NT max = -INFTY;
                 forall_inout_edges(e, u) max = leda_max(w[e], max);
                 pot[u] = max/2;
               } break; }
    case 1:  { 
               free = greedy_matching(G, w, pot, mate, perfect); break; }
    default: { 
               free = jump_start(G, w, pot, mate, perfect); break; }      
  }
  #ifdef _STAT
  heur_t = used_time(heur_t);
  #endif

  if (free == 0) {
    
    forall_edges(e, G) {
      u = source(e);
      v = target(e);
      
      if (mate[u] && (mate[u] == v) &&
          mate[v] && (mate[v] == u))
        M.push(e);
    }
    return M;
  }

  forall_nodes(u, G) {
    item_of[u] = new_blossom(pot[u], u, CUR);

    if (mate[u]) {
      CUR->mate = mate[u];    
      CUR->label = unlabeled;
    }
  }
#ifdef _STAT
  init_t = used_time(init_t);
  alg_t = used_time();
#endif

  forall_nodes(r, G) {
#ifdef MW_MATCHING_DEBUG
    if (debug) cout << "GROW SEARCH TREE rooted at " << index(r) << endl;
#endif
    R = _BLOSSOM_OF(r);
    if (R->mate) continue;

    
    delta1 = delta2a = INFTY;
    delta2b.clear(); delta3.clear(); delta4.clear();
    Q.clear();

    R->status_change(even, Delta, T, Q);

    bool terminate = false;
    while (!terminate) {

      
      #ifdef _STAT
      scan_c++;
      float scan_h = used_time();
      #endif

      NT cur_pot, adj_pot, actual_p, stored_p;

      while (!Q.empty()) {
        cur     = Q.pop();
        CUR     = _BLOSSOM_OF(cur);
        cur_pot = compute_potential(CUR, Delta, item_of[cur]);

        if (!perfect) {
          
          if (cur_pot < delta1 - Delta) {
            delta1 = cur_pot + Delta;
            resp_d1 = cur;
            // if (delta1 == Delta) break; 
          }
        }
        forall_inout_edges(e, cur) {
          adj = opposite(cur, e);     
          ADJ = _BLOSSOM_OF(adj);

          
          // do not consider edges within a blossom
          if (CUR == ADJ) continue;

          // do not consider tree edges
          if ((ADJ->label == odd) &&
              ((ADJ->base == adj && ADJ->mate == cur) ||
               (ADJ->disc == adj && ADJ->pred == cur))) continue;

          adj_pot  = compute_potential(ADJ, Delta, item_of[adj]);
          actual_p = cur_pot + adj_pot - w[e];

      #ifdef MW_MATCHING_DEBUG
        if (debug) {
          cout << "\tSCANNING edge (" << index(cur) << ", " << index(adj) << "): " << 
            cur_pot << " + " << adj_pot << " - " << w[e] << " = " << actual_p << endl;
        }
      #endif
          
          #if !defined(_NO_PRUNING)
          if ((ADJ->label == even) && ADJ->item_in_T) {
            if (actual_p/2 + Delta > leda_min(delta1, delta2a)) continue;
          }
          else if (actual_p + Delta > leda_min(delta1, delta2a)) continue;
          #endif

          if ((ADJ->label == even) && !ADJ->item_in_T) {
            
            if (actual_p < delta2a - Delta) {
              delta2a  = actual_p + Delta;
              resp_d2a = e;
              if (delta2a == Delta) break;    
            } 
          }
          else if (ADJ->label == unlabeled) {  
            
            stored_p = actual_p - ADJ->offset + Delta;
            if (ADJ->improve_connection(item_of[adj], stored_p, cur))
              if (ADJ->item_in_pq)
                delta2b.decrease_p(ADJ->item_in_pq, actual_p + Delta);
              else {
                ADJ->item_in_pq = delta2b.insert(actual_p + Delta, ADJ);
                ADJ->item_in_O = O.append(ADJ);
              } 
          }
          else if (ADJ->label == even)  // ADJ is even tree blossom
            delta3.insert(actual_p/2 + Delta, e);
          else if (ADJ->label == odd) {  
            stored_p = actual_p - ADJ->offset;
            ADJ->improve_connection(item_of[adj], stored_p, cur);
          }
        }
      }

      #ifdef _STAT
      scan_t += used_time(scan_h);
      #endif         
#ifdef MW_MATCHING_DEBUG
    if (debug) {
      map<blossom<NT>*, bool> printed(false);
      blossom<NT>* B;
      forall_nodes(v, G) {
        B = _BLOSSOM_OF(v);
        LABEL label = B->label;
        NT dv;

        dv = compute_potential(B, Delta,item_of[v]);

        cout << "trivial blossom " << index(v) << ": dv: " << dv << "; label: "; 
        if (label == even) cout << "+";
        else if (label == odd) cout << "-";
        else cout << "o";

        if (!B->trivial()) { cout << "; sb: ["; B->print_vertices(); cout << "]"; }
        cout << endl;

        if (!B->trivial() && !printed[B]) {
          printed[B] = true;
          dv = compute_potential(B, Delta);
          cout << "non-trivial blossom ["; B->print_vertices(); cout << "]: dv: " << dv << "; label: "; 
          if (label == even) cout << "+";
          else if (label == odd) cout << "-";
          else cout << "o";
          cout << endl;
        }     
      }
      cout << endl << endl;
    }

    if (debug) print_pqs<NT>(delta1, resp_d1, delta2a, resp_d2a, delta2b,
                             delta3, delta4, Delta, perfect);
#endif
      
      NT cand2b = (delta2b.empty() ? INFTY : delta2b.prio(delta2b.find_min()));
      NT cand3  = (delta3.empty()  ? INFTY : delta3.prio(delta3.find_min()));
      NT cand4  = (delta4.empty()  ? INFTY : delta4.prio(delta4.find_min()));

      if (delta2a == Delta) { 
        #ifdef _STAT
        augment_c++;
        float augment_h = used_time();
        #endif

        e = resp_d2a;
        
        resp = source(e);
        opst = target(e);
        RESP = _BLOSSOM_OF(resp);
        OPST = _BLOSSOM_OF(opst);

        if (!OPST->item_in_T) {
          leda_swap(resp, opst);
          leda_swap(RESP, OPST);
        }
        // invariant: OPST is tree blossom
        #ifdef MW_MATCHING_DEBUG
        if (debug) {
          cout << "\tAUGMENT MATCHING using edge " << index(resp) << ", " << index(opst) << " starting at "; 
          RESP->print_vertices(); cout << endl;
        }
        #endif
        RESP->status_change(odd, Delta, T, Q);
        RESP->pred = opst;
        RESP->disc = resp;
        alternate_path(RESP, item_of);
        #ifdef _STAT
        destroy_c++;
        float destroy_h = used_time();
        #endif
        #ifdef MW_MATCHING_DEBUG
        if (debug) cout << "\tDESTROY SEARCH TREE" << endl;
        #endif

        forall(CUR, T) {
          if (CUR->label == odd) {
            CUR->disc = CUR->pred = nil;
            CUR->item_in_pq = nil;
          }
          CUR->reset();
          CUR->status_change(unlabeled, Delta, T, Q);
        }
        T.clear();

        forall(CUR, O) { 
          CUR->reset(); 
          CUR->item_in_pq = nil;
          CUR->item_in_O = nil; 
        }
        O.clear();
        #ifdef _STAT
        destroy_t += used_time(destroy_h);
        #endif
        #ifdef _STAT
        augment_t += used_time(augment_h);
        #endif
        terminate = true;
      }
      else if (delta1 == Delta) {
        #ifdef _STAT
        alternate_c++;
        float alternate_h = used_time();
        #endif

        RESP = _BLOSSOM_OF(resp_d1);
        #ifdef MW_MATCHING_DEBUG
        if (debug) {
          cout << "\tFLIP EDGES starting at ";
          RESP->print_vertices(); cout << endl;
        }
        #endif
        RESP->base = resp_d1;
        alternate_path(RESP, item_of);
        #ifdef _STAT
        destroy_c++;
        float destroy_h = used_time();
        #endif
        #ifdef MW_MATCHING_DEBUG
        if (debug) cout << "\tDESTROY SEARCH TREE" << endl;
        #endif

        forall(CUR, T) {
          if (CUR->label == odd) {
            CUR->disc = CUR->pred = nil;
            CUR->item_in_pq = nil;
          }
          CUR->reset();
          CUR->status_change(unlabeled, Delta, T, Q);
        }
        T.clear();

        forall(CUR, O) { 
          CUR->reset(); 
          CUR->item_in_pq = nil;
          CUR->item_in_O = nil; 
        }
        O.clear();
        #ifdef _STAT
        destroy_t += used_time(destroy_h);
        #endif
        RESP->label = even;
        #ifdef _STAT
        alternate_t += used_time(alternate_h);
        #endif
        terminate = true;
      }
      else if (cand2b == Delta) { 
        #ifdef _STAT
        grow_c++;
        float grow_h = used_time();
        #endif

        RESP = delta2b.inf(delta2b.find_min());
        delta2b.del_item(RESP->item_in_pq);
        RESP->item_in_pq = nil;

        c_pq_item best = RESP->find_min();
        resp = RESP->node_of(best);
        opst = RESP->best_adj(best);
        #ifdef MW_MATCHING_DEBUG
        if (debug) cout << "\tGROW STEP using edge: (" << index(resp) << ", " << index(opst) << ")" << endl;
        #endif

        RESP->status_change(odd, Delta, T, Q);
        RESP->pred = opst;
        RESP->disc = resp;

        O.del_item(RESP->item_in_O); 
        RESP->item_in_O = nil;

        if (!RESP->trivial())
          RESP->item_in_pq = 
            delta4.insert(compute_potential(RESP, Delta)/2 + Delta, RESP);

        node mate = RESP->mate;
        blossom<NT> *MATE = _BLOSSOM_OF(mate);
        MATE->status_change(even, Delta, T, Q);

        if (MATE->item_in_pq) {
          delta2b.del_item(MATE->item_in_pq);
          MATE->item_in_pq = nil;
          O.del_item(MATE->item_in_O); 
          RESP->item_in_O = nil;
        }
        #ifdef _STAT
        grow_t += used_time(grow_h);
        #endif 
      }
      else if (cand3 == Delta) { 
        #ifdef _STAT
        shrink_c++;
        float shrink_h = used_time();
        #endif

        e = delta3.inf(delta3.find_min());
        delta3.del_min();
        
        resp = source(e);
        opst = target(e);
        RESP = _BLOSSOM_OF(resp);
        OPST = _BLOSSOM_OF(opst);

        if (!OPST->item_in_T) {
          leda_swap(resp, opst);
          leda_swap(RESP, OPST);
        }
        // invariant: OPST is tree blossom
        if (RESP == OPST) continue;  // dead edge encountered;
        #ifdef MW_MATCHING_DEBUG
        if (debug) cout << "\tSHRINK STEP using edge: (" << index(resp) << ", " << index(opst) << ")" << endl;
        #endif

        blossom<NT>       *LCA;
        list<node>         P1, P2;
        list<blossom<NT>*> sub1, sub2;

        
        blossom<NT> *CUR1 = RESP, *CUR2 = OPST;
        CUR1->marker1 = CUR2->marker2 = ++lock;

        P1.push(resp); P2.push(opst);

        while (CUR1->marker2 != lock && CUR2->marker1 != lock && 
               (CUR1->mate != nil || CUR2->mate != nil)) {

          if (CUR1->mate) {
            sub1.push(CUR1); 
            P1.push(CUR1->base); P1.push(CUR1->mate);
            CUR1 = _BLOSSOM_OF(CUR1->mate);
            sub1.push(CUR1); 
            P1.push(CUR1->disc); P1.push(CUR1->pred);
            CUR1 = _BLOSSOM_OF(CUR1->pred);
            CUR1->marker1 = lock;
          }

          if (CUR2->mate) {
            sub2.push(CUR2); 
            P2.push(CUR2->base); P2.push(CUR2->mate);
            CUR2 = _BLOSSOM_OF(CUR2->mate);
            sub2.push(CUR2); 
            P2.push(CUR2->disc); P2.push(CUR2->pred);
            CUR2 = _BLOSSOM_OF(CUR2->pred);
            CUR2->marker2 = lock;
          }
        }
        sub1.push(CUR1); sub2.push(CUR2);

        if (CUR2->marker1 == lock) { // CUR2 is LCA 
          while (sub1.head() != CUR2) {
            sub1.pop(); sub1.pop();
            P1.pop(); P1.pop();
            P1.pop(); P1.pop();    
          }
        }
        else if (CUR1->marker2 == lock) { // CUR1 is LCA
          while (sub2.head() != CUR1) {
            sub2.pop(); sub2.pop();
            P2.pop(); P2.pop();
            P2.pop(); P2.pop();    
          }
        }

        // sub1.head() == sub2.head() == LCA 
        LCA = sub1.pop();
        sub2.reverse(); sub1.conc(sub2);
        P2.reverse(); P1.conc(P2);
              
        blossom<NT> *SUPER   = new blossom<NT>(LCA->base);
        SUPER->pot           = -2*Delta;
        SUPER->mate          = LCA->mate;
        SUPER->shrink_path() = P1;

        forall(CUR, sub1) {  
          if (CUR->item_in_pq) { 
            delta4.del_item(CUR->item_in_pq);
            CUR->item_in_pq = nil;
          }
          SUPER->append_subblossom(CUR, Delta, T, Q);
        }
        SUPER->item_in_T = T.append(SUPER);
        #ifdef _STAT
        shrink_t += used_time(shrink_h);
        #endif 
      }
      else if (cand4 == Delta) {
        #ifdef _STAT
        expand_c++;
        float expand_h = used_time();
        #endif

        RESP = delta4.inf(delta4.find_min());
        delta4.del_item(RESP->item_in_pq);
        #ifdef MW_MATCHING_DEBUG
        if (debug) { 
          cout << "\tEXPAND STEP "; 
          RESP->print_vertices(); cout << endl;
        }
        #endif

        RESP->expand(Delta);
        forall(CUR, RESP->subblossom_p()) 
          CUR->item_in_T = T.append(CUR);
        T.del(RESP->item_in_T);

        blossom<NT> *BASE = _BLOSSOM_OF(RESP->base);
        blossom<NT> *DISC = _BLOSSOM_OF(RESP->disc);

        int dist = RESP->restore_matching(BASE, DISC);
        
        if (dist % 2) {
          RESP->subblossom_p().reverse();
          RESP->shrink_path().reverse();
        }
        else RESP->subblossom_p().push(RESP->subblossom_p().Pop());

        DISC->disc = RESP->disc;
        DISC->pred = RESP->pred;

        CUR = RESP->subblossom_p().pop();

        while (CUR != DISC) {
          ADJ = RESP->subblossom_p().pop();
          cur = RESP->shrink_path().pop();
          adj = RESP->shrink_path().pop();

          CUR->pred = adj; CUR->disc = cur;

          if (!CUR->trivial())
            CUR->item_in_pq = 
              delta4.insert(compute_potential(CUR, Delta)/2 + Delta, CUR);
          ADJ->status_change(even, Delta, T, Q);

          RESP->shrink_path().pop();
          RESP->shrink_path().pop();

          CUR = RESP->subblossom_p().pop();
        }
        // send item for DISC also
        if (!CUR->trivial())
          CUR->item_in_pq = 
            delta4.insert(compute_potential(CUR, Delta)/2 + Delta, CUR);

        while (!RESP->subblossom_p().empty()) {
          CUR = RESP->subblossom_p().pop();
          CUR->status_change(unlabeled, Delta, T, Q);

          if (!CUR->empty()) {
            CUR->item_in_pq = delta2b.insert(CUR->min_prio() + CUR->offset, CUR);
            CUR->item_in_O = O.append(CUR);
          }
          CUR->pred = CUR->disc = nil;
        }
        RESP->destroy();

        #ifdef _STAT
        expand_t += used_time(expand_h);
        #endif
      }
      else {
        #ifdef _STAT
        adj_c++;
        #endif

        NT delta = leda_min(delta1, 
                            leda_min(delta2a, 
                                     leda_min(cand2b, 
                                              leda_min(cand3, cand4))));

        if ((delta == INFTY) && perfect) {
#ifdef _STAT
  alg_t = used_time(alg_t);
#endif
          // clean-up (which was missing in the original code by G. Schäfer)
          // we destroy all remaining blossoms
          forall_nodes(v, G) {
            slist<blossom<NT>*> blossom_queue;
            blossom<NT>* bl_v = _BLOSSOM_OF(v);
		    if (bl_v) blossom_queue.append(bl_v);
            while (! blossom_queue.empty()) {
              blossom<NT>* bl = blossom_queue.pop();
              if (bl->has_subblossoms()) {
                blossom<NT>* sbl;
			    forall(sbl, bl->subblossom_p()) blossom_queue.append(sbl);
			  }
              bl->destroy();
            }
          }

          return M;   // return empty matching
        }
        #ifdef MW_MATCHING_DEBUG  
          if (debug) {
            cout << "\tPERFORMED DUAL ADJUSTMENT by " << delta-Delta << endl;
            cout << "\tVALUE OF DELTA: " << delta << endl;
          }
        #endif
        Delta = delta;   // corresponds to Delta += (delta - Delta) 
      }
    }
  }
#ifdef _STAT
  alg_t = used_time(alg_t);
#endif

  #ifdef _STAT
    extract_t = used_time();
  #endif

  int k = 0;
  forall_nodes(v, G) 
    unpack_blossom(_BLOSSOM_OF(v), item_of, pot, b, BT, k, -1, Delta);
  if (k != 0) BT.resize(k);

  forall_edges(e, G)
    if (_BLOSSOM_OF(source(e))->mate == target(e)) M.push(e);
  #ifdef _STAT
    extract_t = used_time(extract_t);
  #endif

  forall_nodes(v, G) 
    _BLOSSOM_OF(v)->destroy();

#ifdef MW_MATCHING_DEBUG
  if (debug) {
    cout << "Matching: " << endl;
    forall(e, M) {
      u = source(e);
      v = target(e);
      cout << "(" << index(u) << ", " << index(v) << ")" << endl;
    }
  }
#endif
  return M;
}

/// MST APPROACH ////////////////////////////////////////////////////////////////////////////

#else // #ifdef _SST_APPROACH

// ----- #include "MST.t"



#define _MAP_IMPL h_array


#ifdef _STAT
int   adj_c, 
      alternate_c, grow_c, shrink_c, expand_c, augment_c, destroy_c,
      scan_c;             
float alternate_t, grow_t, shrink_t, expand_t, augment_t, destroy_t, 
      scan_t;
float heur_t, init_t, alg_t, extract_t; 
#endif


template<class NT> class blossom;
template<class NT> class vertex; 
template<class NT> class tree;

template<class NT> 
inline c_pq_item new_blossom(NT d, node b, blossom<NT>* &B) {
  B = new blossom<NT>(b);
  vertex<NT> *v = new vertex<NT>(d, b);
  return B->init(MAX_VALUE(NT), v);
}

template<class NT> 
inline blossom<NT>* blossom_of(c_pq_item it, NT) {
  return (it ? (blossom<NT>*)(get_owner(it)) : nil);
}

template<class NT> 
class blossom : public /*virtual*/ concat_pq<NT, vertex<NT>*> 
{ /* MST */
  typedef concat_pq<NT, vertex<NT>*> base_class;
  
  struct info {
    list<node>         shrink_path;
    list<blossom<NT>*> subblossom_p; 
    c_pq_item          split_item;

	info() : split_item(nil) {}
	LEDA_MEMORY(info)
  };
  info* _info;
  info& get_info() { if (!_info) _info = new info; return *_info; }

public:
  
  LABEL   label;
  bool    min_changed;

  NT      pot;
  NT      offset;

  node base, mate;
  node disc, pred;

  double marker1, marker2;

  list_item item_in_T;
  pq_item   item_in_pq;

  tree<NT> *my_tree;
  
  list<node>&         shrink_path() { return get_info().shrink_path; }
  list<blossom<NT>*>& subblossom_p() { return get_info().subblossom_p;}
  c_pq_item&          split_item() { return get_info().split_item; }

  bool has_subblossoms() const { return _info != nil && !_info->subblossom_p.empty(); }

  blossom(node ba = nil) : concat_pq<NT, vertex<NT>*>() {

    set_owner(leda_cast(this));
    label = even;
    pot = offset = 0; 
    base = ba; mate = nil;
    disc = pred = nil;
    marker1 = marker2 = 0;
    min_changed = false;
    my_tree = nil;
    item_in_T = nil;
    item_in_pq = nil;

	_info = nil;
  }

  ~blossom() { kill(); }

  void kill() {
    base_class::clear();
	delete _info; _info = nil;
  }

  void destroy() {
    c_pq_item it;
    forall_items(it, *this)
      delete this->inf(it);
    this->kill();
    delete this;
  }

  NT min_prio() const 
  { return (base_class::find_min() ? prio(base_class::find_min()) : MAX_VALUE(NT)); }

  NT   pot_of(c_pq_item it)        const { return base_class::inf(it)->pot;       }
  node node_of(c_pq_item it)  const { return base_class::inf(it)->my_node;   }

  void expand(NT Delta) {
      
    blossom<NT> *CUR;
    forall(CUR, subblossom_p()) {
      split_at_item(CUR->split_item(), *CUR, *this);
      CUR->offset = offset;
      CUR->label = label;

      if (!CUR->trivial() && label == odd)
        CUR->pot += 2*offset + 2*Delta;
      else if (!CUR->trivial()) {
        assert(!CUR->item_in_T);
        assert(CUR->label == even || CUR->label == unlabeled);
        CUR->pot += 2*offset;
      }
    }
  }

  bool trivial() const { return base_class::size() == 1; }

  void print_vertices() const {
    vertex<NT> *Cur;
    forall(Cur, *this) cout << index(Cur->my_node) << "  ";
  }

  node best_adj(c_pq_item it) const { return base_class::inf(it)->min_inf(); }

  node tree_root() const { return (my_tree ? my_tree->root : nil); } 

  void status_change(LABEL l, NT Delta, node_slist &Q) {

    if (l == unlabeled) {
      assert((label != l) && item_in_T);
      offset += (label == odd ? Delta : -Delta);
      my_tree->remove(this);
    }
    else if (l == odd) {
      assert((label != l) && !item_in_T);
      offset -= Delta;
      my_tree->add(this);
    }
    else if (l == even) {
      assert((label != l) || !item_in_T);
      if (label == odd) offset += 2*Delta;
      else {  // non-tree blossom
        offset += Delta;
        my_tree->add(this);
      }  
      
      c_pq_item it;
      forall_items(it, *this) {
        Q.append(node_of(it));  
        delete_connection(it, my_tree->root);
        
        #ifdef _PROVIDENT
        if (offset != 0) {
          inf(it)->pot += offset;
          if (inf(it)->empty()) continue;
          inf(it)->adjust_priorities(offset);
          increase_p(it, prio(it) + offset);
        }
        #endif
      }
      
      #ifdef _PROVIDENT
      if (!trivial()) pot -= 2*offset;
      offset = 0;
      #endif
    }
    label = l;
  }

  bool delete_connection(c_pq_item it, node r) {

    if (!it) return false;
    NT old_min = min_prio();
    if (base_class::inf(it)->del(r))
      if (base_class::inf(it)->empty()) 
        base_class::del_item(it);
      else
        increase_p(it, base_class::inf(it)->min_prio());
    return old_min != min_prio();
  }

  bool improve_connection(c_pq_item it, NT x, node u, node r) {

    if (!it) return false;
    NT old_min = min_prio();
    if (base_class::inf(it)->decrease_p1(u, x, r)) decrease_p(it, x);    
    return old_min != min_prio();    
  }

  void delete_all_connections(const node_array<c_pq_item> &item_of, 
                              slist<blossom<NT>*> &correct) {

    edge e;
    c_pq_item it;
    node cur, adj;
    blossom<NT> *ADJ; 

    forall_items(it, *this) {
      cur = node_of(it);
      forall_inout_edges(e, cur) {
        adj = opposite(cur, e);
        ADJ = blossom_of(item_of[adj],NT(0)); 

        bool m_changed = ADJ->delete_connection(item_of[adj], tree_root());     

        if (m_changed && !ADJ->min_changed && ADJ->label != odd) {
          correct.append(ADJ);
          ADJ->min_changed = true;
        }
      }
    }
  }

  void best_edge(node &resp, node &opst) const {
    resp = node_of(base_class::find_min());
    opst = best_adj(base_class::find_min());
  }

  void append_subblossom(blossom<NT>* CUR, NT Delta, node_slist &Q) {

    if (CUR->label == odd) 
      CUR->status_change(even, Delta, Q);

    if (!CUR->trivial())
      CUR->pot += -2*CUR->offset + 2*Delta;

    if (offset != CUR->offset) {
      
      blossom<NT>* SMALL_B = (base_class::size() < CUR->size() ? this : CUR);
      blossom<NT>* LARGE_B = (base_class::size() < CUR->size() ? CUR : this);

      NT adjustment = SMALL_B->offset - LARGE_B->offset;

      c_pq_item it;
      forall_items(it, *SMALL_B) {       

        SMALL_B->inf(it)->pot += adjustment;
        if (SMALL_B->inf(it)->empty()) continue;

        SMALL_B->inf(it)->adjust_priorities(adjustment);
        NT cur_prio = SMALL_B->prio(it);
        if (adjustment < 0) 
          SMALL_B->decrease_p(it, cur_prio + adjustment);
        else 
          SMALL_B->increase_p(it, cur_prio + adjustment);
      }
      offset = LARGE_B->offset;    
    }

    CUR->my_tree->remove(CUR);
    concat(*CUR);
    CUR->split_item() = base_class::last_item();
    subblossom_p().append(CUR);   
  }

  int restore_matching(blossom<NT> *BASE, blossom<NT> *DISC) {

    
    while (subblossom_p().tail() != BASE) {
      subblossom_p().append(subblossom_p().pop());
      shrink_path().append(shrink_path().pop());
      shrink_path().append(shrink_path().pop());
    }
    
    BASE->mate = mate;
    BASE->base = base;

    node ba, m;
    int dist = 0, pos = 1;
    list_item p_it   = shrink_path().first();
    list_item sub_it = subblossom_p().first();
    blossom<NT> *CUR = subblossom_p().inf(sub_it), *ADJ;

    while (CUR != BASE) {
      if (CUR == DISC) dist = pos;
      sub_it = subblossom_p().succ(sub_it); pos++;
      ADJ    = subblossom_p().inf(sub_it);
      if (ADJ == DISC) dist = pos; 

      p_it = shrink_path().succ(p_it); 
      p_it = shrink_path().succ(p_it); ba = shrink_path().inf(p_it);
      p_it = shrink_path().succ(p_it); m = shrink_path().inf(p_it);

      CUR->base = ba; CUR->mate = m;
      ADJ->base = m; ADJ->mate = ba;
         
      sub_it = subblossom_p().succ(sub_it); pos++;
      CUR    = subblossom_p().inf(sub_it);
      p_it   = shrink_path().succ(p_it);
    }
    if (CUR == DISC) dist = pos; 
    return dist;
  }

  LEDA_MEMORY(blossom<NT>);
};

template<class NT> 
class vertex : public /*virtual*/ p_queue<NT, node> { /* MST */

typedef typename p_queue<NT,node>::item pq_item;
typedef p_queue<NT,node> base_class;

public:

  NT   pot;
  node my_node;  
  h_array<node, pq_item> ITEM_OF;

   
  vertex(NT d, node u) : p_queue<NT, node>(MWM_INIT_PQ_SZ), ITEM_OF(nil, MWM_INIT_PQ_SZ)
  { pot = d; my_node = u; }
  ~vertex() { ITEM_OF.clear(); base_class::clear(); }

  NT   min_prio() const 
  { return (base_class::find_min() ? prio(base_class::find_min()) : MAX_VALUE(NT)); }

  node min_inf() const 
  { return (base_class::find_min() ? inf(base_class::find_min()) : nil); }

  bool decrease_p1(node u, NT x, node r) {
    
    NT old_min = min_prio();
    if (!ITEM_OF.defined(r)) 
      ITEM_OF[r] = insert(x, u);
    else {
      pq_item it = ITEM_OF[r];
      if (prio(it) > x) {
        p_queue<NT, node>::decrease_p(it, x);
        p_queue<NT, node>::change_inf(it, u);
      }
    }
    return old_min != min_prio();
  }

  bool del(node r) {

    if (!ITEM_OF.defined(r)) return false;
    NT old_min = min_prio();
    del_item(ITEM_OF[r]);
    ITEM_OF.undefine(r);
    return old_min != min_prio();
  }
    
  void adjust_priorities(NT adjustment) {

    if (adjustment == 0) return;

    node r;
    pq_item it;
    NT cur_prio;
    forall_defined(r, ITEM_OF) {
      it = ITEM_OF[r];
      cur_prio = prio(it);
      if (adjustment < 0)
        p_queue<NT, node>::decrease_p(it, cur_prio + adjustment);
      else {  // simulate increase_p
        node v = inf(it);
        del_item(it);
        ITEM_OF[r] = insert(cur_prio + adjustment, v);
      }
    }
  }

  LEDA_MEMORY(vertex<NT>);
};


template<class NT> 
inline tree<NT>* new_tree(node r, blossom<NT>* &B) {
  tree<NT>* T = new tree<NT>(r); 
  B->item_in_T = T->my_blossoms.append(B);
  return T;
}

template<class NT> class tree { /* MST */
  
public:

  node root;
  node d1_node;

  list<blossom<NT>*> my_blossoms;

  p_queue<NT, edge>  d3b_edges;
  pq_item            item_in_d3b;

  
  tree(node r = nil) : d3b_edges(MWM_INIT_PQ_SZ) { root = r; d1_node = nil; item_in_d3b = nil; }
  ~tree() {}

  void add(blossom<NT> *B) 
  { B->item_in_T = my_blossoms.append(B); B->my_tree = this; }

  void remove(blossom<NT> *B) 
  { my_blossoms.del(B->item_in_T); B->item_in_T = nil; B->my_tree = nil; }

  NT min_prio() const { 
    return (d3b_edges.find_min() ? \
            d3b_edges.prio(d3b_edges.find_min()) : MAX_VALUE(NT)); }

  edge min_inf() const 
  { return (d3b_edges.find_min() ? \
            d3b_edges.inf(d3b_edges.find_min()) : nil); }

  bool ins(NT x, edge e) {
    pq_item old_min = d3b_edges.find_min();
    d3b_edges.insert(x, e);
    return (old_min != d3b_edges.find_min());
  }

  void destroy_tree(slist<blossom<NT>*> &correct, 
                    node_pq<NT> &delta1, 
                    p_queue<NT, blossom<NT>*> &delta3a, 
                    p_queue<NT, tree<NT>*> &delta3b, 
                    p_queue<NT, blossom<NT>*> &delta4,
                    NT Delta, node_slist &Q, const node_array<c_pq_item> &item_of) {
  #ifdef _STAT
    destroy_c++;
    float destroy_h = used_time();
  #endif

    blossom<NT>* CUR;
    forall(CUR, my_blossoms) {
      if (CUR->label == even) 
        CUR->delete_all_connections(item_of, correct);

      
      if (CUR->item_in_pq) {
        if (CUR->label == even) 
          delta3a.del_item(CUR->item_in_pq);
        else 
          delta4.del_item(CUR->item_in_pq);
        CUR->item_in_pq = nil;
      }

      if (!CUR->min_changed) {
        correct.push(CUR);
        CUR->min_changed = true;
      }
      CUR->pred = CUR->disc = nil;
      CUR->status_change(unlabeled, Delta, Q);
    }
    
    if (d1_node)
      delta1.del(d1_node);
    if (item_in_d3b) 
      delta3b.del_item(item_in_d3b);
    delete this;
  #ifdef _STAT
    destroy_t += used_time(destroy_h);
  #endif
  }

  void del_dead_edges(const node_array<c_pq_item> &item_of) {

    blossom<NT> *CUR, *ADJ;
    while (!d3b_edges.empty()) {
      edge e = min_inf();

      CUR = blossom_of(item_of[source(e)],NT(0));
      ADJ = blossom_of(item_of[target(e)],NT(0));

      if (CUR != ADJ) break;
      else d3b_edges.del_min();
    }
  }

  LEDA_MEMORY(tree<NT>);
};

#define _BLOSSOM_OF(this_node) \
(this_node ? blossom_of(item_of[this_node],NT(0)) : nil)

template<class NT>
inline NT compute_potential(blossom<NT> *CUR, NT Delta, c_pq_item it) {

  int a = (it == nil ? -2 : 1);
  int sigma = 0;
  if (CUR->item_in_T) sigma = (CUR->label == even ? -1 : 1);
  NT stored = (it == nil ? CUR->pot : CUR->pot_of(it));

  return stored + a * CUR->offset + a * sigma * Delta;
}

// sn: still some compilers do not support default arguments for 
//     template functions

template<class NT>
inline NT compute_potential(blossom<NT> *CUR, NT Delta) {
    return compute_potential(CUR,Delta, (c_pq_item)nil);
}

 /* MST */
template<class NT> 
__temp_func_inline
void alternate_path(blossom<NT>* RESP, const node_array<c_pq_item> &item_of) {

  if (!RESP) return;
#ifdef MW_MATCHING_DEBUG
if (debug) {cout << "\tAUGMENT PATH" << endl; }
#endif

  blossom<NT> *CUR = RESP;
  node pred = RESP->base, disc = nil;

  while (CUR) {
    if (CUR->label == even) {
      // father of CUR in the tree is its mate
      node mate = CUR->mate;
      CUR->mate = disc;
      CUR->base = pred;
      CUR = _BLOSSOM_OF(mate);
    }
    else {  // CUR->label == odd
      // father of CUR in the tree is given by pred
      pred = CUR->pred;
      disc = CUR->disc;
      CUR->mate = pred;
      CUR->base = disc;
      CUR = _BLOSSOM_OF(pred);    
    }
  }
}

template<class NT> 
__temp_func_inline
void correct_pqs(slist<blossom<NT>*> &correct, 
                 p_queue<NT, blossom<NT>*> &delta2,
                 p_queue<NT, blossom<NT>*> &delta3a) {

#ifdef MW_MATCHING_DEBUG
  if (debug) { cout << "\tCORRECTING delta pqs" << endl;  }
#endif
  blossom<NT> *CUR;

  forall(CUR, correct) {
#ifdef MW_MATCHING_DEBUG
    if (debug) {
      cout << "\t\tadjusting delta for blossom: "; 
      CUR->print_vertices(); 
      cout << endl; 
    }
#endif

    if (CUR->item_in_pq) {
      
      if (!CUR->item_in_T)
        delta2.del_item(CUR->item_in_pq);
      else  
        delta3a.del_item(CUR->item_in_pq);
      CUR->item_in_pq = nil;
    }
    if (!CUR->empty()) {
      
      if (!CUR->item_in_T)
        CUR->item_in_pq = delta2.insert(CUR->min_prio() + CUR->offset, CUR);
      else 
        CUR->item_in_pq = delta3a.insert((CUR->min_prio() + CUR->offset)/2, CUR);    
    }
    CUR->min_changed = false;
  }
  correct.clear();
}

template<class NT> 
__temp_func_inline
void unpack_blossom(blossom<NT> *RESP, const node_array<c_pq_item> &item_of,
                    node_array<NT> &pot, node_array<int> &b, 
                    array<two_tuple<NT, int> > &BT,
                    int &k, int parent, NT Delta) {

  if (RESP->trivial()) {
    
    node cur = RESP->node_of(RESP->first_item());
    if (b[cur] != -1) return;
    pot[cur] = compute_potential(RESP, Delta, item_of[cur]);
    b[cur] = parent;      
    #ifdef MW_MATCHING_DEBUG
      if (debug) cout << "trivial blossom " << index(cur) << ": dv:" << pot[cur] 
                      << " immediate super blossom: " << b[cur] << endl;
    #endif
  }
  else {
    
    if (k > BT.high()) BT.resize(2*k+1);
    BT[k].first()  = compute_potential(RESP, Delta);
    BT[k].second() = parent;
    int idx = k++;
    #ifdef MW_MATCHING_DEBUG
      if (debug) {
        cout << "non-trivial blossom "; RESP->print_vertices();
        cout << ": dv: " << BT[idx].first() << ", index: " << idx << " and parent blossom " 
             << BT[idx].second() << endl;
      }
    #endif

    RESP->expand(Delta);
    blossom<NT> *BASE = _BLOSSOM_OF(RESP->base);
    blossom<NT> *DISC = nil;
    RESP->restore_matching(BASE, DISC);

    blossom<NT>* CUR;
    forall(CUR, RESP->subblossom_p()) 
      unpack_blossom(CUR, item_of, pot, b, BT, k, idx, Delta);
                     
    // mod. 12/09/00
    RESP->destroy();
  }
}

#if defined(_CHECK) || defined(MW_MATCHING_DEBUG)

template<class NT> 
__temp_func_inline
void restore_matching(blossom<NT>* bl, blossom<NT>* BASE,
					  map<blossom<NT>*, node>& Mate, map<blossom<NT>*, node>& Base)
{
	list<blossom<NT>*> _subblossom_p = bl->subblossom_p();
	list<node>         _shrink_path  = bl->shrink_path();

    while (_subblossom_p.tail() != BASE) {
      _subblossom_p.append(_subblossom_p.pop());
      _shrink_path.append(_shrink_path.pop());
      _shrink_path.append(_shrink_path.pop());
    }
    
    Mate[BASE] = Mate[bl];
    Base[BASE] = Base[bl];

    node ba, m;
    list_item p_it   = _shrink_path.first();
    list_item sub_it = _subblossom_p.first();
    blossom<NT> *CUR = _subblossom_p.inf(sub_it), *ADJ;

    while (CUR != BASE) {
      sub_it = _subblossom_p.succ(sub_it);
      ADJ    = _subblossom_p.inf(sub_it);

      p_it = _shrink_path.succ(p_it); 
      p_it = _shrink_path.succ(p_it); ba = _shrink_path.inf(p_it);
      p_it = _shrink_path.succ(p_it); m =  _shrink_path.inf(p_it);

      Base[CUR] = ba; Mate[CUR] = m;
      Base[ADJ] = m;  Mate[ADJ] = ba;
         
      sub_it = _subblossom_p.succ(sub_it);
      CUR    = _subblossom_p.inf(sub_it);
      p_it   = _shrink_path.succ(p_it);
    } 
}

template<class NT> 
__temp_func_inline
void extract_subblossom(blossom<NT>* surface_bl, 
						blossom<NT>* RESP, c_pq_item start_RESP, c_pq_item end_RESP, 
					    const node_array<c_pq_item> &item_of,
                        node_array<NT> &pot, node_array<int> &b, 
                        array<two_tuple<NT, int> > &BT,
                        int &k, int parent, NT Delta,
						map<blossom<NT>*, node>& Mate, map<blossom<NT>*, node>& Base,
						map<c_pq_item, blossom<NT>*>& Owner)
{
  if (start_RESP == end_RESP) { // RESP is trivial
    node cur = surface_bl->node_of(start_RESP);
	int idx = index(cur);
    if (b[cur] != -1) return;
    pot[cur] = compute_potential(surface_bl, Delta, item_of[cur]);
    b[cur] = parent;    
  }
  else {
    if (k > BT.high()) BT.resize(2*k+1);
    BT[k].first()  = compute_potential(surface_bl, Delta);
    BT[k].second() = parent;
    int idx = k++;

/* simulate!
    RESP->expand(Delta); // NOTE: This changes the owner cf. _BLOSSOM_OF
    blossom<NT> *BASE = _BLOSSOM_OF(RESP->base);
    blossom<NT> *DISC = nil;
    RESP->restore_matching(BASE, DISC);
*/

	// simulate expand
    c_pq_item start = start_RESP;
    blossom<NT>* CUR;
	forall(CUR, RESP->subblossom_p()) {
	  c_pq_item end = CUR->split_item();
	  c_pq_item after_end = surface_bl->next_item(end);
      for(c_pq_item cur = start; cur != after_end; cur = surface_bl->next_item(cur))
		  Owner[cur] = CUR;
      extract_subblossom(surface_bl, CUR, start, end, item_of, pot, b, BT, k, idx, Delta, Mate, Base, Owner);
	  start = after_end;
	}

	// simulate restore
	c_pq_item item_of_base = item_of[RESP->base];
	blossom<NT>* BASE = Owner.defined(item_of_base) ? Owner[item_of_base] : (blossom<NT>*) get_owner(item_of_base);
	restore_matching(RESP, BASE, Mate, Base);
  }
}

template<class NT> 
__temp_func_inline
void extract_matching_and_proof(list<edge>& M,
                    node_array<NT> &pot, 
					node_array<int> &b, array<two_tuple<NT, int> > &BT,
					const graph& G,
					const node_array<c_pq_item> &item_of,
                    NT Delta)
{
  M.clear();
  pot.init(G);
  b.init(G, -1);
  BT.resize(0);

/* destructive extraction:
  static unsigned cnt; ++cnt;
  if (cnt >= 10) {
    int k = 0;
	node v;
    forall_nodes(v, G) 
      unpack_blossom(_BLOSSOM_OF(v), item_of, pot, b, BT, k, -1, Delta);
    if (k != 0) BT.resize(k);

	edge e;
    forall_edges(e, G)
      if (_BLOSSOM_OF(source(e))->mate == target(e)) M.push(e);
    return;
  }
*/

  map<blossom<NT>*, node> Mate(nil), Base(nil);
  map<c_pq_item, blossom<NT>*> Owner(nil);
	// new owner of items that will change their owner during a blossom->expand operation

  node v;
  forall_nodes(v, G) {
    blossom<NT>* B = _BLOSSOM_OF(v);
	if (!B) continue;
    Mate[B] = B->mate;
	Base[B] = B->base;
  }

  int k = 0;
  map<blossom<NT>*, bool> handled(false);
  forall_nodes(v, G) {
    blossom<NT>* B = _BLOSSOM_OF(v);
	if (handled[B]) continue;
    extract_subblossom(B, B, B->first_item(), B->last_item(), item_of, pot, b, BT, k, -1, Delta, Mate, Base, Owner);
	handled[B] = true;
  }
  if (k != 0) BT.resize(k);

  edge e;
  forall_edges(e, G) {
	c_pq_item it = item_of[source(e)];
	blossom<NT>* src_bl = Owner.defined(it) ? Owner[it] : (blossom<NT>*) get_owner(it);
    if (Mate[src_bl] == target(e)) M.push(e);
  }
}
#endif

#ifdef MW_MATCHING_DEBUG
template<class NT> 
__temp_func_inline
void show_matching_and_proof(const graph& G,
					const node_array<c_pq_item>& item_of,
                    NT Delta)
{
	list<edge> M;
	node_array<NT> pot;
	node_array<int> b;
	array<two_tuple<NT, int> > BT;
	extract_matching_and_proof(M, pot, b, BT, G, item_of, Delta);

	static unsigned cnt; ++cnt;
    cout << "step " << cnt << " / matching: " << endl;
	edge e;
    forall(e, M) {
	  cout << index(source(e)) << "<->" << index(target(e)) << " ";
    }
    cout << endl;

    cout <<"pot: " << endl;
    node n;
    forall_nodes(n, G) cout << index(n) << ":" << pot[n] << " ";
    cout << endl;

	if (BT.size() > 0) {
      cout <<"BT: " << endl;
	  for (int k = BT.low(); k <= BT.high(); ++k) {
	    cout << k << ": (" << BT[k].first() << "," << BT[k].second() << ") ";
	  }
	  cout << endl;

      cout <<"b: " << endl;
      forall_nodes(n, G) if (b[n] != -1) cout << index(n) << ":" << b[n] << " ";
      cout << endl;
	}
}
#endif

#ifdef _CHECK
template<class NT> 
__temp_func_inline
bool CHECK_MWM_T(const graph &G, const edge_array<NT> &w,
                 const list<edge> &M, const node_array<NT> &pot,
                 const array<two_tuple<NT, int> > &BT,
                 const node_array<int> &b, 
                 bool perfect, bool check_optimality, 
				 const char* s0);

template<class NT> 
__temp_func_inline
void check_invariants(const graph& G, const edge_array<NT>& w,
					  const node_array<c_pq_item>& item_of,
                      NT Delta)
{
	list<edge> M;
	node_array<NT> pot;
	node_array<int> b;
	array<two_tuple<NT, int> > BT;
	extract_matching_and_proof(M, pot, b, BT, G, item_of, Delta);
	static unsigned cnt = 0; ++cnt;
	string msg = string("check invariants MWM (%d): ", cnt); 
	assert( CHECK_MWM_T(G, w, M, pot, BT, b, false, false, msg) );
}
#endif

template<class NT>
__temp_func_inline
void print_pqs(node_pq<NT> &delta1, 
               p_queue<NT, blossom<NT>*> &delta2,
               p_queue<NT, blossom<NT>*> &delta3a, 
               p_queue<NT, tree<NT>*> &delta3b,
               p_queue<NT, blossom<NT>*> &delta4,
               const NT Delta, bool perfect) {

  pq_item it;

  if (!perfect) {
    cout << "delta1: " << endl;
    list<node> L;
    delta1.get_nodes(L);
    node v;
    forall(v, L) {
      cout << delta1.prio(v) - Delta << "\t"
           << index(v) << endl;
    }
  }

  cout << "delta2: " << endl;
  forall_items(it, delta2) {
    cout << delta2.prio(it) - Delta << "\t";
    delta2.inf(it)->print_vertices(); cout << endl;
  }

  cout << "delta3a: " << endl;
  forall_items(it, delta3a) {
    cout << delta3a.prio(it) - Delta << "\t"; 
    delta3a.inf(it)->print_vertices(); cout << endl;
  }

  cout << "delta3b: " << endl;
  forall_items(it, delta3b)
    cout << delta3b.prio(it) - Delta << "\t"
         << index(delta3b.inf(it)->root) << endl;

  cout << "delta4: " << endl;
  forall_items(it, delta4) {
    cout << delta4.prio(it) - Delta << "\t";
    delta4.inf(it)->print_vertices(); cout << endl;
  }
}

template<class NT> 
__temp_func_inline
list<edge> MWM_MST(const graph &G, const edge_array<NT> &w, 
                   node_array<NT> &pot, array<two_tuple<NT, int> > &BT, 
                   node_array<int> &b, int heur/* =1*/, bool perfect/* =false*/)
{
#ifdef _STAT
  adj_c = alternate_c = grow_c = shrink_c = expand_c = augment_c = scan_c = destroy_c = 0;
  heur_t = init_t = alg_t = extract_t = alternate_t = grow_t = shrink_t = \
expand_t = augment_t = scan_t = destroy_t = 0;
  init_t = used_time();
#endif

  //cout << sizeof(conc_pq_node<2,16>) << endl;
  //cout << sizeof(conc_pq_tree<>) << endl;
  //cout << sizeof(concat_pq<NT, int>) << endl;
  //cout << sizeof(blossom<NT>) << endl;
  //cout << sizeof(vertex<NT>) << endl;
  
  edge        e;
  node        resp, opst, cur,  adj, u, v /*, r */;
  blossom<NT> *RESP, *OPST, *CUR, *ADJ /*, *U, *V, *R */;

  list<edge>  M;

  const NT INFTY = MAX_VALUE(NT);

  node_array<c_pq_item> item_of(G); // after the initialization this is read-only
  node_slist Q(G);

  NT Delta = 0;
  double lock = 0;

  node_pq<NT> delta1(G);             // contains for each tree an even labeled node with min. potential

  p_queue<NT, blossom<NT>*> delta2;  // contains each non-tree blossom with the reduced cost of its best connection to a tree (-> augment)

  p_queue<NT, blossom<NT>*> delta3a; // contains even tree blossoms with (half) the reduced cost of their best connection to a different tree (-> augment)
  p_queue<NT, tree<NT>*>    delta3b; // contains even tree blossoms with (half) the reduced cost of their best connection to the same tree (-> shrink)

  p_queue<NT, blossom<NT>*> delta4;  // contains odd tree blossoms with (half) their potential
  int free = G.number_of_nodes(); 
  
  node_array<node> mate(G, nil);

  #ifdef _STAT
  heur_t = used_time();
  #endif
  switch(heur) {
    case 0:  { 
               forall_nodes(u, G) {
                 if (degree(u) == 0) { pot[u] = 0; continue; }
                 NT max = -INFTY;
                 forall_inout_edges(e, u) max = leda_max(w[e], max);
                 pot[u] = max/2;
               } break; }
    case 1:  { 
               free = greedy_matching(G, w, pot, mate, perfect); break; }
    default: { 
               free = jump_start(G, w, pot, mate, perfect); break; }      
  }
  #ifdef _STAT
  heur_t = used_time(heur_t);
  #endif

  if (free == 0) {

	forall_edges(e, G) {
      u = source(e);
      v = target(e);
      
      if (mate[u] && (mate[u] == v) /*&& mate[v] && (mate[v] == u)*/)
        M.push(e);
    }
  #ifdef _STAT
    init_t = used_time(init_t);
  #endif
    return M; // no blossoms -> BT remains empty, b is -1 for all nodes
  }

  forall_nodes(u, G) {
    item_of[u] = new_blossom(pot[u], u, CUR);

    if (mate[u]) {
      CUR->mate = mate[u];    
      CUR->label = unlabeled;
    }
    else {
      CUR->my_tree = new_tree(u, CUR);
      Q.append(u);
    }
  }
#ifdef _STAT
  init_t = used_time(init_t);
  alg_t = used_time();
#endif
#ifdef MW_MATCHING_DEBUG
  if (debug) cout << "START MATCHING ROUTINE..." << endl;
#endif

  while (free) {
	#ifdef MW_MATCHING_DEBUG
    if (debug) show_matching_and_proof(G, item_of, Delta);
    #endif
	#ifdef _CHECK
    check_invariants(G, w, item_of, Delta);
    #endif

    #ifdef _STAT
    scan_c++;
    float scan_h = used_time();
    #endif

    NT cur_pot, adj_pot, actual_p, stored_p;

    while (!Q.empty()) {
      cur     = Q.pop();
      CUR     = _BLOSSOM_OF(cur);
      cur_pot = compute_potential(CUR, Delta, item_of[cur]);

      if (!perfect) {
        
        if (!CUR->my_tree->d1_node) {
          delta1.insert(cur, cur_pot + Delta);
          CUR->my_tree->d1_node = cur;
        }
        else if (cur_pot < delta1.prio(CUR->my_tree->d1_node) - Delta) {
          delta1.del(CUR->my_tree->d1_node);
          delta1.insert(cur, cur_pot + Delta);
          CUR->my_tree->d1_node = cur;  
        }
      }
      forall_inout_edges(e, cur) {
        adj = opposite(cur, e);     
        ADJ = _BLOSSOM_OF(adj);

        
        // do not consider edges within a blossom
        if (CUR == ADJ) continue;

        // do not consider tree edges
        if ((ADJ->label == odd) &&
            ((ADJ->base == adj && ADJ->mate == cur) ||
             (ADJ->disc == adj && ADJ->pred == cur))) continue;

        adj_pot  = compute_potential(ADJ, Delta, item_of[adj]);
        actual_p = cur_pot + adj_pot - w[e];
    #ifdef MW_MATCHING_DEBUG
      if (debug) {
        cout << "\tSCANNING edge (" << index(cur) << ", " << index(adj) << "): " << 
          cur_pot << " + " << adj_pot << " - " << w[e] << " = " << actual_p << endl;
      }
    #endif

        if (!ADJ->item_in_T) {
          
          stored_p = actual_p - ADJ->offset + Delta;
          if (ADJ->improve_connection(item_of[adj], stored_p, cur, CUR->tree_root()))
            if (ADJ->item_in_pq)
              delta2.decrease_p(ADJ->item_in_pq, actual_p + Delta);
            else
              ADJ->item_in_pq = delta2.insert(actual_p + Delta, ADJ);
        }
        else if ((ADJ->label == even) && (ADJ->my_tree != CUR->my_tree)) {
          
          stored_p = actual_p - ADJ->offset + 2*Delta;
          if (ADJ->improve_connection(item_of[adj], stored_p, cur, CUR->tree_root())) {
            if (ADJ->item_in_pq)
              delta3a.decrease_p(ADJ->item_in_pq, actual_p/2 + Delta);
            else 
              ADJ->item_in_pq = delta3a.insert(actual_p/2 + Delta, ADJ);  
          }
        }
        else if ((ADJ->label == even) && (ADJ->my_tree == CUR->my_tree)) {
          
          tree<NT> *T = CUR->my_tree;
          if (T->ins(actual_p/2 + Delta, e))
            if (T->item_in_d3b)
              delta3b.decrease_p(T->item_in_d3b, actual_p/2 + Delta);
            else 
              T->item_in_d3b = delta3b.insert(actual_p/2 + Delta, T);
        }
        else if (ADJ->label == odd) {
          stored_p = actual_p - ADJ->offset;
          ADJ->improve_connection(item_of[adj], stored_p, cur, CUR->tree_root());
        }
      }
    } // while (!Q.empty())
    #ifdef _STAT
    scan_t += used_time(scan_h);
    #endif       
#ifdef MW_MATCHING_DEBUG
    if (debug) {
      map<blossom<NT>*, bool> printed(false);
      blossom<NT>* B;
      forall_nodes(v, G) {
        B = _BLOSSOM_OF(v);
        LABEL label = B->label;
        NT dv = compute_potential(B, Delta, item_of[v]);
        cout << "trivial blossom " << index(v) << ": dv: " << dv << "; label: "; 
        if (label == even) cout << "+";
        else if (label == odd) cout << "-";
        else cout << "o";

        if (!B->trivial()) { cout << "; sb: ["; B->print_vertices(); cout << "]"; }
        cout << endl;

        if (!B->trivial() && !printed[B]) {
          printed[B] = true;
          dv = compute_potential(B, Delta);
          cout << "non-trivial blossom ["; B->print_vertices(); cout << "]: dv: " << dv << "; label: "; 
          if (label == even) cout << "+";
          else if (label == odd) cout << "-";
          else cout << "o";
          cout << endl;
        }     
      }
      cout << endl << endl;
    }

    if (debug) print_pqs<NT>(delta1, delta2, delta3a, delta3b, delta4, Delta, perfect);
#endif
        
    NT cand1  = (delta1.empty()  ? INFTY : delta1.prio(delta1.find_min()));
    NT cand2  = (delta2.empty()  ? INFTY : delta2.prio(delta2.find_min()));
    NT cand3a = (delta3a.empty() ? INFTY : delta3a.prio(delta3a.find_min()));
    NT cand3b = (delta3b.empty() ? INFTY : delta3b.prio(delta3b.find_min()));
    NT cand4  = (delta4.empty()  ? INFTY : delta4.prio(delta4.find_min()));

    if (cand3a == Delta) {
      // augment step: tree (OPST) <-> tree (RESP)
      #ifdef _STAT
      augment_c++;
      float augment_h = used_time();
      #endif

      RESP = delta3a.inf(delta3a.find_min());
      delta3a.del_item(RESP->item_in_pq);
      RESP->item_in_pq = nil;

      RESP->best_edge(resp, opst);
      OPST = _BLOSSOM_OF(opst);
      #ifdef _CHECK
      assert( OPST->my_tree != nil && RESP->my_tree != nil );
      #endif

      alternate_path(RESP, item_of);
      alternate_path(OPST, item_of);
      RESP->base = OPST->mate = resp; 
      RESP->mate = OPST->base = opst;

      slist<blossom<NT>*> correct;
      RESP->my_tree->destroy_tree(correct, delta1, delta3a, 
                                  delta3b, delta4, Delta, Q, item_of);
      OPST->my_tree->destroy_tree(correct, delta1, delta3a, 
                                  delta3b, delta4, Delta, Q, item_of);
      correct_pqs(correct, delta2, delta3a);
      #ifdef _STAT
      augment_t += used_time(augment_h);
      #endif  
      free -= 2;
    } 
    else if (cand1 == Delta) {
      #ifdef _STAT
      alternate_c++;
      float alternate_h = used_time();
      #endif

      resp = delta1.del_min();
      RESP = _BLOSSOM_OF(resp);
      RESP->base = resp;
      #ifdef MW_MATCHING_DEBUG
      if (debug) { cout << "\tALTERNATE STEP for blossom: "; RESP->print_vertices(); cout << endl; }
      #endif
      alternate_path(RESP, item_of);

      slist<blossom<NT>*> correct;
      RESP->my_tree->destroy_tree(correct, delta1, delta3a, 
                                  delta3b, delta4, Delta, Q, item_of);
      correct_pqs(correct, delta2, delta3a);
      RESP->label = even;
      #ifdef _STAT
      alternate_t = used_time(alternate_h);
      #endif
      free -= 1;
    }
    else if (cand2 == Delta) {     
      
      RESP = delta2.inf(delta2.find_min());
      delta2.del_item(RESP->item_in_pq);
      RESP->item_in_pq = nil;

      if (RESP->label == even) {
        // augment step: tree (OBST) <-> single node (RESP)
        #ifdef _STAT
        augment_c++;
        float augment_h = used_time();
        #endif

        RESP->best_edge(resp, opst);
        OPST = _BLOSSOM_OF(opst);
        #ifdef _CHECK
        assert( OPST->my_tree != nil && RESP->my_tree == nil );
        #endif

        alternate_path(OPST, item_of);
        RESP->base = OPST->mate = resp; 
        RESP->mate = OPST->base = opst;
        RESP->label = unlabeled;

        slist<blossom<NT>*> correct;
        OPST->my_tree->destroy_tree(correct, delta1, delta3a, 
                                    delta3b, delta4, Delta, Q, item_of);
		correct.append(RESP); // ST: the connections of RESP have to be updated too!
        correct_pqs(correct, delta2, delta3a);
        free -= 1;
        #ifdef _STAT
        augment_t += used_time(augment_h);
        #endif
      }
      else {
        #ifdef _STAT
        grow_c++;
        float grow_h = used_time();
        #endif

        RESP->best_edge(resp, opst);
        OPST = _BLOSSOM_OF(opst);
        #ifdef MW_MATCHING_DEBUG
        if (debug) cout << "\tGROW STEP using edge: (" << index(resp) << ", " << index(opst) << ")" << endl;
        #endif

        RESP->my_tree = OPST->my_tree;
        RESP->status_change(odd, Delta, Q);
        RESP->pred = opst;
        RESP->disc = resp;
        if (!RESP->trivial())
          RESP->item_in_pq = 
            delta4.insert(compute_potential(RESP, Delta)/2 + Delta, RESP);

        node mate = RESP->mate;
        blossom<NT> *MATE = _BLOSSOM_OF(mate);
        MATE->my_tree = OPST->my_tree;
        MATE->status_change(even, Delta, Q);
        if (MATE->item_in_pq) {
          delta2.del_item(MATE->item_in_pq);
          if (!MATE->empty()) 
            MATE->item_in_pq = 
              delta3a.insert((MATE->min_prio() + MATE->offset)/2, MATE);
          else MATE->item_in_pq = nil;
        }
        #ifdef _STAT
        grow_t += used_time(grow_h);
        #endif
      }
    }
    else if (cand3b == Delta) {
      #ifdef _STAT
      shrink_c++;
      float shrink_h = used_time();
      #endif

      tree<NT> *T = delta3b.inf(delta3b.find_min());
      delta3b.del_item(T->item_in_d3b);
      T->item_in_d3b = nil;

      e = T->min_inf();
      T->d3b_edges.del_min();
      resp = source(e); RESP = _BLOSSOM_OF(resp);
      opst = target(e); OPST = _BLOSSOM_OF(opst);
      #ifdef MW_MATCHING_DEBUG
      if (debug) cout << "\tSHRINK STEP using edge: (" << index(resp)
                      << ", " << index(opst) << ")"<< endl;
      #endif

      blossom<NT>       *LCA;
      list<node>         P1, P2;
      list<blossom<NT>*> sub1, sub2;

      
      blossom<NT> *CUR1 = RESP, *CUR2 = OPST;
      CUR1->marker1 = CUR2->marker2 = ++lock;

      P1.push(resp); P2.push(opst);

      while (CUR1->marker2 != lock && CUR2->marker1 != lock && 
             (CUR1->mate != nil || CUR2->mate != nil)) {

        if (CUR1->mate) {
          sub1.push(CUR1); 
          P1.push(CUR1->base); P1.push(CUR1->mate);
          CUR1 = _BLOSSOM_OF(CUR1->mate);
          sub1.push(CUR1); 
          P1.push(CUR1->disc); P1.push(CUR1->pred);
          CUR1 = _BLOSSOM_OF(CUR1->pred);
          CUR1->marker1 = lock;
        }

        if (CUR2->mate) {
          sub2.push(CUR2); 
          P2.push(CUR2->base); P2.push(CUR2->mate);
          CUR2 = _BLOSSOM_OF(CUR2->mate);
          sub2.push(CUR2); 
          P2.push(CUR2->disc); P2.push(CUR2->pred);
          CUR2 = _BLOSSOM_OF(CUR2->pred);
          CUR2->marker2 = lock;
        }
      }
      sub1.push(CUR1); sub2.push(CUR2);

      if (CUR2->marker1 == lock) { // CUR2 is LCA 
        while (sub1.head() != CUR2) {
          sub1.pop(); sub1.pop();
          P1.pop(); P1.pop();
          P1.pop(); P1.pop();    
        }
      }
      else if (CUR1->marker2 == lock) { // CUR1 is LCA
        while (sub2.head() != CUR1) {
          sub2.pop(); sub2.pop();
          P2.pop(); P2.pop();
          P2.pop(); P2.pop();    
        }
      }

      // sub1.head() == sub2.head() == LCA 
      LCA = sub1.pop();
      sub2.reverse(); sub1.conc(sub2);
      P2.reverse(); P1.conc(P2);

      blossom<NT> *SUPER   = new blossom<NT>(LCA->base);
      SUPER->mate          = LCA->mate;
      SUPER->my_tree       = T;
      SUPER->shrink_path() = P1;

      forall(CUR, sub1) {
        if (CUR->item_in_pq) { 
          if (CUR->label == odd)
            delta4.del_item(CUR->item_in_pq);
          else 
            delta3a.del_item(CUR->item_in_pq);
          CUR->item_in_pq = nil;
        }
        SUPER->append_subblossom(CUR, Delta, Q);
      }

      SUPER->pot = 2*(SUPER->offset - Delta);
      T->add(SUPER);
      if (!SUPER->empty()) 
        SUPER->item_in_pq = 
          delta3a.insert((SUPER->min_prio() + SUPER->offset)/2, SUPER);

      T->del_dead_edges(item_of);
      if (!T->d3b_edges.empty())
        T->item_in_d3b = delta3b.insert(T->min_prio(), T);
      #ifdef _STAT
      shrink_t += used_time(shrink_h);
      #endif
    }
    else if (cand4 == Delta) {
      #ifdef _STAT
      expand_c++;
      float expand_h = used_time();
      #endif

      RESP = delta4.inf(delta4.find_min());
      delta4.del_item(RESP->item_in_pq);
      #ifdef MW_MATCHING_DEBUG
        if (debug) { 
          cout << "\tEXPAND STEP"; 
          RESP->print_vertices(); 
          cout << endl;
        }
      #endif

      RESP->expand(Delta);
      forall(CUR, RESP->subblossom_p()) 
        RESP->my_tree->add(CUR);
      RESP->my_tree->remove(RESP);

      blossom<NT> *BASE = _BLOSSOM_OF(RESP->base);
      blossom<NT> *DISC = _BLOSSOM_OF(RESP->disc);

      int dist = RESP->restore_matching(BASE, DISC);
      
      if (dist % 2) {
        RESP->subblossom_p().reverse();
        RESP->shrink_path().reverse();
      }
      else RESP->subblossom_p().push(RESP->subblossom_p().Pop());

      DISC->disc = RESP->disc;
      DISC->pred = RESP->pred;

      CUR = RESP->subblossom_p().pop();

      while (CUR != DISC) {
        ADJ = RESP->subblossom_p().pop();
        cur = RESP->shrink_path().pop();
        adj = RESP->shrink_path().pop();

        CUR->pred = adj; CUR->disc = cur;

        if (!CUR->trivial())
          CUR->item_in_pq = 
            delta4.insert(compute_potential(CUR, Delta)/2 + Delta, CUR);

        ADJ->status_change(even, Delta, Q);

        if (!ADJ->empty()) 
          ADJ->item_in_pq = delta3a.insert((ADJ->min_prio() + ADJ->offset)/2, ADJ);

        RESP->shrink_path().pop();
        RESP->shrink_path().pop();
        CUR = RESP->subblossom_p().pop();
      }
      // send item for DISC also
      if (!CUR->trivial())
        CUR->item_in_pq = 
          delta4.insert(compute_potential(CUR, Delta)/2 + Delta, CUR);

      while (!RESP->subblossom_p().empty()) {
        CUR = RESP->subblossom_p().pop();
        CUR->status_change(unlabeled, Delta, Q);

        if (!CUR->empty()) 
          CUR->item_in_pq = delta2.insert(CUR->min_prio() + CUR->offset, CUR);

        CUR->pred = CUR->disc = nil;
      }
      RESP->destroy();
      #ifdef _STAT
      expand_t += used_time(expand_h);
      #endif
    }
    else {
      
      #ifdef _STAT
      adj_c++;
      #endif
      NT delta = leda_min(cand1, 
                          leda_min(cand2, 
                                   leda_min(cand3a, 
                                            leda_min(cand3b, cand4))));

      if ((delta == INFTY) && perfect) {
#ifdef _STAT
  alg_t = used_time(alg_t);
#endif
        // clean-up (which was missing in the original code by G. Schäfer)
        // we destroy all remaining blossoms and the search trees containing them
        map<tree<NT>*, tree<NT>*> trees; // we abuse the map as a set of pointers ...
        forall_nodes(v, G) {
          slist<blossom<NT>*> blossom_queue;
          blossom<NT>* bl_v = _BLOSSOM_OF(v);
		  if (bl_v) blossom_queue.append(bl_v);
		  while (! blossom_queue.empty()) {
            blossom<NT>* bl = blossom_queue.pop();
			if (bl->has_subblossoms()) {
			  blossom<NT>* sbl;
			  forall(sbl, bl->subblossom_p()) blossom_queue.append(sbl);
			}
            trees[bl->my_tree] = bl->my_tree; // just to remember the tree
            bl->destroy();
		  }
        }
        tree<NT>* t;
        forall(t, trees) delete t;

	    return M;   // return empty matching
      }
      #ifdef MW_MATCHING_DEBUG  
        if (debug) {
          cout << "\tPERFORMED DUAL ADJUSTMENT by " << delta-Delta << endl;
          cout << "\tVALUE OF DELTA: " << delta << endl;
        }
      #endif
      Delta = delta;   // corresponds to Delta += (delta - Delta)
    }
  }
#ifdef _STAT
  alg_t = used_time(alg_t);
#endif

  // >>> extraction of proof (i.e. potentials) and matching
  #ifdef _STAT
    extract_t = used_time();
  #endif

  int k = 0;
  forall_nodes(v, G) 
    unpack_blossom(_BLOSSOM_OF(v), item_of, pot, b, BT, k, -1, Delta);
  if (k != 0) BT.resize(k);

  forall_edges(e, G)
    if (_BLOSSOM_OF(source(e))->mate == target(e)) M.push(e);

  #ifdef _STAT
    extract_t = used_time(extract_t);
  #endif
  // <<< extraction of proof (i.e. potentials) and matching

  forall_nodes(v, G) 
    _BLOSSOM_OF(v)->destroy();

#ifdef MW_MATCHING_DEBUG
  if (debug) {
    cout << "Matching: " << endl;
    forall(e, M) {
      u = source(e);
      v = target(e);
      cout << "(" << index(u) << ", " << index(v) << ")" << endl;
    }
  }
#endif
  return M;
}

#endif // #ifdef _SST_APPROACH, #else (i.e. _MST) branch


// ----- interface functions


float check_t, total_t;

bool APPLIED_AUTO_SCALING = false;


template <class NT>
__temp_func_inline
bool max_absolute_value(const graph& G, const edge_array<NT>& w, NT &max) { 
  // mod. 03/2002 by GS

  bool neg_weights = false;
  max = 0;
  edge e;
  forall_edges(e, G) { 
    if ( w[e] < 0 ) neg_weights = true;
    if (  w[e] > max ) max =  w[e];
    if ( -w[e] > max ) max = -w[e];
  }
  return neg_weights;
}


template<class NT> 
__temp_func_inline
bool check_weights(const graph &G, edge_array<NT> &w, bool perfect, string s) {
  // mod. 03/2002 by GS

  edge e;
  bool res = true;
  leda_assert(LEDA_TYPE_ID(NT) == INT_TYPE_ID, s + "non-integral weights are not fully supported yet");
  if (!res) return res;

  bool no_scaling = true;
  forall_edges(e, G) 
    if (w[e] != (w[e]/4)*4) { no_scaling = false; break; }

  if (!no_scaling) {
    APPLIED_AUTO_SCALING = true;
    leda_assert(no_scaling, s + "weights must be multiples of 4; autoscaling applied");
    forall_edges(e, G) w[e] *= 4;
  } 
  else APPLIED_AUTO_SCALING = false;

  NT C_max;
  bool neg_weights = max_absolute_value(G, w, C_max);
 
  if (!perfect) {
    res = res && leda_assert(!neg_weights, s + "weights must be non-negative");
    leda_assert(C_max <= MAX_VALUE(NT)/10, s + "danger of overflow");
  }
  else {
    leda_assert(C_max <= MAX_VALUE(NT)/G.number_of_nodes(), s + "danger of overflow");
  }
  return res;
}


template<class NT> 
__temp_func_inline
bool CHECK_WEIGHTS_T(const graph &G, edge_array<NT> &w, bool perfect) {

  return check_weights(G, w, perfect, "CHECK_WEIGHTS(...): ");
}


template<class NT> 
__temp_func_inline
bool is_feasible_instance(const graph &G, edge_array<NT> &w, bool perfect, string s) {

  bool res = true;

  res = res && leda_assert(Is_Simple(G), s + "graph must be simple");
  res = res && leda_assert(Is_Loopfree(G), s + "graph must be loopfree");
  res = res && leda_assert(Is_Undirected_Simple(G), s + "graph must be undirected (no anti-parallel edges)");
  res = res && check_weights(G, w, perfect, s);
  return res;
}


template<class NT> 
__temp_func_inline
list<edge> MAX_WEIGHT_MATCHING_T(const graph &G, 
                                 const edge_array<NT> &w, 
                                 bool check, int heur) {

  node_array<NT>  pot(G);
  node_array<int> b(G, -1);
  array<two_tuple<NT, int> > BT; 

  return MAX_WEIGHT_MATCHING_T(G, w, pot, BT, b, check, heur);
}



template<class NT> 
__temp_func_inline
list<edge> MAX_WEIGHT_MATCHING_T(const graph &G, 
                                 const edge_array<NT> &w0, 
                                 node_array<NT> &pot, 
                                 array<two_tuple<NT, int> > &BT, 
                                 node_array<int> &b,
                                 bool check, int heur) {
  edge e;
  edge_array<NT> w(G);  
  forall_edges(e, G) w[e] = w0[e];

  list<edge> M;
  if (!is_feasible_instance(G, w, false, "MAX_WEIGHT_MATCHING: ")) 
    return M;

  pot.init(G);
  b.init(G, -1);
  BT = array<two_tuple<NT, int> >();

#ifdef _INFO
  cout << "COMPUTE MAX WEIGHT MATCHING..." << endl;
#endif
  total_t = used_time();
#if defined(_SST_APPROACH)
  M = MWM_SST(G, w, pot, BT, b, heur, false);
#else
  M = MWM_MST(G, w, pot, BT, b, heur, false);
#endif
  total_t = used_time(total_t);
#ifdef _INFO
  printf("READY! TOTAL TIME: %7.2f sec.\n", total_t);
#endif

#ifdef _INFO
  if (check) cout << "CHECK MAX WEIGHT MATCHING..." << flush;
#endif
  check_t = used_time();
  if (check) assert(CHECK_MAX_WEIGHT_MATCHING_T(G, w, M, pot, BT, b));
  check_t = used_time(check_t);
#ifdef _INFO
  if (check) printf("OK! CHECKER TIME: %7.2f sec.\n", check_t);
#endif

  return M;
}



template<class NT> 
__temp_func_inline
list<edge> MAX_WEIGHT_PERFECT_MATCHING_T(const graph &G, 
                                         const edge_array<NT> &w, 
                                         bool check, int heur) {


  node_array<NT>  pot(G);
  node_array<int> b(G, -1);
  array<two_tuple<NT, int> > BT; 

  return MAX_WEIGHT_PERFECT_MATCHING_T(G, w, pot, BT, b, check, heur);
}



template<class NT> 
__temp_func_inline
list<edge> MAX_WEIGHT_PERFECT_MATCHING_T(const graph &G, 
                                         const edge_array<NT> &w0, 
                                         node_array<NT> &pot, 
                                         array<two_tuple<NT, int> > &BT, 
                                         node_array<int> &b,
                                         bool check, int heur) {

  edge e;
  edge_array<NT> w(G);  
  forall_edges(e, G) w[e] = w0[e];

  list<edge> M;
  if (!is_feasible_instance(G, w, true, "MAX_WEIGHT_PERFECT_MATCHING: ")) 
    return M;

  pot.init(G);
  b.init(G, -1);
  BT = array<two_tuple<NT, int> >();  

#ifdef _INFO
  cout << "COMPUTE MAX WEIGHT PERFECT MATCHING..." << endl;
#endif
  total_t = used_time();
#if defined(_SST_APPROACH)
  M = MWM_SST(G, w, pot, BT, b, heur, true);
#else
  M = MWM_MST(G, w, pot, BT, b, heur, true);
#endif
  total_t = used_time(total_t);
#ifdef _INFO
  printf("READY! TOTAL TIME: %7.2f sec.\n", total_t);
#endif

#ifdef _INFO
  if (check) cout << "CHECK MAX WEIGHT PERFECT MATCHING..." << flush;
#endif
  check_t = used_time();
  if (check && M.length()) assert(CHECK_MAX_WEIGHT_PERFECT_MATCHING_T(G, w, M, pot, BT, b));
  check_t = used_time(check_t);
#ifdef _INFO
  if (check) printf("OK! CHECKER TIME: %7.2f sec.\n", check_t);
#endif

  return M;
}


template<class NT> 
__temp_func_inline
list<edge> MIN_WEIGHT_PERFECT_MATCHING_T(const graph &G, 
                                         const edge_array<NT> &w, 
                                         bool check, int heur) {


  node_array<NT>  pot(G);
  node_array<int> b(G, -1);
  array<two_tuple<NT, int> > BT; 

  return MIN_WEIGHT_PERFECT_MATCHING_T(G, w, pot, BT, b, check, heur);
}



template<class NT> 
__temp_func_inline
list<edge> MIN_WEIGHT_PERFECT_MATCHING_T(const graph &G, 
                                         const edge_array<NT> &w0, 
                                         node_array<NT> &pot, 
                                         array<two_tuple<NT, int> > &BT, 
                                         node_array<int> &b,
                                         bool check, int heur) {

  edge e;
  edge_array<NT> w(G);  
  forall_edges(e, G) w[e] = w0[e];

  list<edge> M;
  if (!is_feasible_instance(G, w, true, "MIN_WEIGHT_PERFECT_MATCHING: ")) 
    return M;

  edge_array<NT> w_mod(G);
  forall_edges(e, G) w_mod[e] = -w[e];

  pot.init(G);
  b.init(G, -1);
  BT = array<two_tuple<NT, int> >();  

#ifdef _INFO
  cout << "COMPUTE MIN WEIGHT PERFECT MATCHING..." << endl;
#endif
  total_t = used_time();
#if defined(_SST_APPROACH)
  M = MWM_SST(G, w_mod, pot, BT, b, heur, true);
#else
  M = MWM_MST(G, w_mod, pot, BT, b, heur, true);
#endif
  total_t = used_time(total_t);
#ifdef _INFO
  printf("READY! TOTAL TIME: %7.2f sec.\n", total_t);
#endif

#ifdef _INFO
  if (check) cout << "CHECK MIN WEIGHT PERFECT MATCHING..." << flush;
#endif
  check_t = used_time();
  if (check && M.length()) assert(CHECK_MIN_WEIGHT_PERFECT_MATCHING_T(G, w, M, pot, BT, b));
  check_t = used_time(check_t);
#ifdef _INFO
  if (check) printf("OK! CHECKER TIME: %7.2f sec.\n", check_t);
#endif

  return M;
}




template<class NT> 
__temp_func_inline
bool CHECK_MWM_T(const graph &G, const edge_array<NT> &w,
                 const list<edge> &M, const node_array<NT> &pot,
                 const array<two_tuple<NT, int> > &BT,
                 const node_array<int> &b, 
                 bool perfect, bool check_optimality, 
				 const char* s0)
{
  string s = s0;

  // empty matching
  if (perfect && M.length() == 0) return false;

  node v;
  edge e;
  int K = BT.size();
  bool res = true;
  int i;

  // M is a (perfect) matching
  node_array<int> deg_in_M(G,0);

  forall(e, M) { 
    deg_in_M[G.source(e)]++;
    deg_in_M[G.target(e)]++;
  }

  forall_nodes(v, G)
    if (perfect) 
      res = res && leda_assert(deg_in_M[v] == 1, s + "M is not a perfect matching");
    else
      res = res && leda_assert(deg_in_M[v] <= 1, s + "M is not a matching");

  // check semantics of b and BT
  forall_nodes(v, G)
    res = res && leda_assert(b[v] < K && b[v] >= -1, s + "failed test -1 <= b[v] < K");

  for(i = 0; i< K; i++) 
    res = res && leda_assert(BT[i].first() >= 0, s + "failed test BT[i].first() >= 0");

  for(i = 0; i < K; i++) 
    res = res && leda_assert(BT[i].second() < i, s + "failed test BT[i].second() < i");

  // determine nearest common ancestor
  edge_array<int> nearest_common_ancestor(G, -1);

  forall_edges(e, G) {
    node i = G.source(e);
    node j = G.target(e);
    int bi = b[i];
    int bj = b[j];

    while (bi != bj && bi >= 0 && bj >= 0) {
      if (bi < bj) 
	bj = BT[bj].second();
      else 
	bi = BT[bi].second();
    }
    if (bi == bj && bi >= 0)
      nearest_common_ancestor[e] = bi;
  }

  // every blossom must contain a maximum number of matching edges
  array<int> number_of_matched_in_blossom(K);
  array<int> size_of_blossom(K);

  for (i = 0; i < K; i++)
    size_of_blossom[i] = number_of_matched_in_blossom[i] = 0;

  forall_nodes(v, G)
    if (b[v] != -1)
      size_of_blossom[b[v]]++;

  forall(e, M)
    if (nearest_common_ancestor[e] != -1)
      number_of_matched_in_blossom[nearest_common_ancestor[e]]++;

  for(i = K - 1; i >= 0; i--) {
    int j = BT[i].second();

    if (j >= 0) {
      size_of_blossom[j] += size_of_blossom[i];
      number_of_matched_in_blossom[j] += number_of_matched_in_blossom[i];
    }
    res = res && leda_assert(BT[i].first() == 0 || 
                             2 * number_of_matched_in_blossom[i] + 1 == size_of_blossom[i],
                             s + "failed test: blossoms must contain maximum number of matching edges");
  }

  // non-negative potentials in non-perfect case
  if (!perfect) 
    forall_nodes (v, G) 
      leda_assert(pot[v] >= 0, s + "failed test pot[v] >= 0");

  // compute reduced costs
  array<two_tuple<NT, int> > BT_local;
  BT_local = BT;
  edge_array<NT> pi(G, 0);

  for (i = 0; i < K; i++) {
    int j = BT_local[i].second();
    if (j >= 0)
      BT_local[i].first() += BT_local[j].first();
  }

  forall_edges (e, G) {
    node i = G.source(e);
    node j = G.target(e);
    if (i != j) {
      pi[e] = pot[i] + pot[j] - w[e];
      int k = nearest_common_ancestor[e];
      if (k >= 0) pi[e] += BT_local[k].first();
    }
  }

  // non-negative reduced costs
  forall_edges(e, G) 	
    res = res && leda_assert(pi[e] >= 0, s + "pi[e] >= 0 does not hold for edge e = (" 
                                           + string("%i", index(source(e))) + ", " + string("%i", index(target(e))) + ")");
  // matching edges are tight
  forall(e, M)
    res = res && leda_assert(pi[e] == 0, s + "pi[e] == 0 does not hold for edge e = (" 
                                           + string("%i", index(source(e))) + ", " + string("%i", index(target(e))) + ")");

  // check optimality
  if (check_optimality && !perfect) {
    forall_nodes(v, G)
	  res = res && leda_assert(deg_in_M[v] == 1 || pot[v] == 0, 
	                           s + string("node %i is free, but has non-neg. potential", index(v)));
  }

  return res;
}




template<class NT> 
__temp_func_inline
bool CHECK_MAX_WEIGHT_MATCHING_T(const graph &G,
			         const edge_array<NT> &w,
			         const list<edge> &M,
			         const node_array<NT> &pot,
			         const array<two_tuple<NT, int> > &BT,
			         const node_array<int> &b) {

  return CHECK_MWM_T(G, w, M, pot, BT, b, false, true, "CHECK_MAX_WEIGHT_MATCHING: ");
}




template<class NT> 
__temp_func_inline
bool CHECK_MAX_WEIGHT_PERFECT_MATCHING_T(const graph &G,
	  			         const edge_array<NT> &w,
				         const list<edge> &M,
				         const node_array<NT> &pot,
				         const array<two_tuple<NT, int> > &BT,
				         const node_array<int> &b) {

  return CHECK_MWM_T(G, w, M, pot, BT, b, true, true, "CHECK_MAX_WEIGHT_PERFECT_MATCHING: ");
}



template<class NT> 
__temp_func_inline
bool CHECK_MIN_WEIGHT_PERFECT_MATCHING_T(const graph &G,
	  			         const edge_array<NT> &w,
				         const list<edge> &M,
				         const node_array<NT> &pot,
				         const array<two_tuple<NT, int> > &BT,
				         const node_array<int> &b) {

  edge_array<NT> w_mod(G); edge e;
  forall_edges(e, G) w_mod[e] = -w[e];

  return CHECK_MWM_T(G, w_mod, M, pot, BT, b, true, true, "CHECK_MIN_WEIGHT_PERFECT_MATCHING: ");
}

LEDA_END_NAMESPACE

#undef _SST_APPROACH
#undef MWM_INIT_PQ_SZ

#if LEDA_ROOT_INCL_ID == 500365
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif
