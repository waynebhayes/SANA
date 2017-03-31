/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  ab1_tree.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:45 $

#ifndef LEDA_AB1_TREE_H
#define LEDA_AB1_TREE_H

//------------------------------------------------------------------------------
// (a,b)-trees 
//------------------------------------------------------------------------------

#include<LEDA/system/basic.h>

LEDA_BEGIN_NAMESPACE

class __exportC ab1_tree_node;

class __exportC ab1_tree_elem {

GenPtr key;
ab1_tree_node* child;

friend class __exportC ab1_tree;
friend class __exportC ab1_tree_node;

};


class __exportC ab1_tree_node {

friend class __exportC ab1_tree;

  static leda_mutex mutex_id_count;
  static unsigned long id_count;

  int height;
  int size;
  unsigned long id;

  ab1_tree_node* father;

  ab1_tree_elem* last;
  ab1_tree_elem  arr[1];

  ab1_tree_node*& succ() { return arr[0].child; }
  ab1_tree_node*& pred() { return arr[1].child; }

  GenPtr& key() { return arr[0].key; }
  GenPtr& inf() { return arr[1].key; }

friend unsigned long ID_Number(ab1_tree_node* p) { return p->id; }

friend inline ab1_tree_node* new_ab1_tree_node(int,int,ab1_tree_node*,int);

}; 
  


class __exportC ab1_tree   
{
    int a;
    int b;

    int height;             /* height of tree   */
    int count;              /* number of leaves */

    ab1_tree_node* root;
    ab1_tree_node* minimum;  
    ab1_tree_node* maximum;

    ab1_tree_elem* expand(ab1_tree_node*, ab1_tree_node*, ab1_tree_node*, int);
    ab1_tree_elem* index(ab1_tree_node* v, ab1_tree_node* u);
    ab1_tree_elem* same(ab1_tree_node* v);

    GenPtr shrink(ab1_tree_node* v, ab1_tree_elem* pos);

    void split_node(ab1_tree_node* v);
    void share(ab1_tree_node* v,ab1_tree_node* y,int direct);
    void fuse (ab1_tree_node* v,ab1_tree_node* w);
    void del_tree(ab1_tree_node* localroot);
    void exchange_leaves(ab1_tree_node*, ab1_tree_node*);
    void flip_leaves(ab1_tree_node*, ab1_tree_node*, ab1_tree_node*);
    void pr_ab1_tree(ab1_tree_node*,int) const;

    void remove_item(ab1_tree_node*);


    ab1_tree_node* copy_ab1_tree(ab1_tree_node*,ab1_tree_node*&,int) const;

    virtual int key_type_id() const { return UNKNOWN_TYPE_ID; }

    virtual int cmp(GenPtr, GenPtr) const { return 0; }
    virtual void clear_key(GenPtr&) const {}
    virtual void clear_inf(GenPtr&) const {}
    virtual void copy_key(GenPtr&)  const {}
    virtual void copy_inf(GenPtr&)  const {}
    virtual void print_key(GenPtr)  const {}
    virtual void print_inf(GenPtr)  const {}




public:

    typedef ab1_tree_node* item;

    void clear();

    const GenPtr& key(ab1_tree_node* p)  const { return p->key(); }
    GenPtr& inf(ab1_tree_node* p)        const { return p->inf(); }

    ab1_tree_node* insert(GenPtr, GenPtr);
    ab1_tree_node* insert_item_at_item(ab1_tree_node*,ab1_tree_node*, int);
    ab1_tree_node* insert_at_item(ab1_tree_node*, GenPtr, GenPtr, int);
    ab1_tree_node* insert_at_item(ab1_tree_node*, GenPtr, GenPtr);
    ab1_tree_node* locate_succ(GenPtr) const;
    ab1_tree_node* locate_pred(GenPtr) const;
    ab1_tree_node* locate(GenPtr,bool&) const;
    ab1_tree_node* locate(GenPtr x) const { return locate_succ(x); }
    ab1_tree_node* lookup(GenPtr) const;

    void del(GenPtr);
    void del_item(ab1_tree_node*);
    void change_inf(ab1_tree_node*, GenPtr);
    void reverse_items(ab1_tree_node*, ab1_tree_node*);

    void conc(ab1_tree&,int);
    void split_at_item(ab1_tree_node*, ab1_tree&, ab1_tree&,int);

    void del_min() { if (minimum) del_item(minimum); }

    void decrease_key(ab1_tree_node* p, GenPtr k);

    bool member(GenPtr k)  const { return (lookup(k))? true: false ; }

    ab1_tree_node* min()                      const { return minimum; }
    ab1_tree_node* find_min()                 const { return minimum; }
    ab1_tree_node* max()                      const { return maximum; }
    ab1_tree_node* first_item()               const { return minimum; }
    ab1_tree_node* last_item()                const { return maximum; }
    ab1_tree_node* next_item(ab1_tree_node* p) const { return p ? p->succ() : 0; }
    ab1_tree_node* succ(ab1_tree_node* p)      const { return p->succ(); }
    ab1_tree_node* pred(ab1_tree_node* p)      const { return p->pred(); }

    ab1_tree_node* stl_next_item(ab1_tree_node* p) const 
    { return p ? p->succ() : minimum; }

    ab1_tree_node* stl_pred_item(ab1_tree_node* p) const 
    { return p ? p->pred() : maximum; }

    int  size()  const { return count;       }
    bool empty() const { return (count==0) ? true : false;   }
    void print() const { pr_ab1_tree(root,1); }

    ab1_tree(int a=2, int b=16); 

    ab1_tree(const ab1_tree& T);

    ab1_tree& operator=(const ab1_tree&);
 
    virtual ~ab1_tree(){ clear(); }
 };

LEDA_END_NAMESPACE

#endif
