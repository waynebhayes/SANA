/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  b_heap1.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:45 $

#ifndef LEDA_BHEAP_H
#define LEDA_BHEAP_H

//------------------------------------------------------------------------------
// b_heap1: bounded heaps with integer keys in [a..b]
//------------------------------------------------------------------------------

#include <LEDA/system/basic.h>
#include <LEDA/core/list.h>
#include <LEDA/core/array.h>

LEDA_BEGIN_NAMESPACE

class __exportC b_heap1_node {

friend class __exportC b_heap1;
friend void print_b_heap1_item(b_heap1_node*);

int key;
GenPtr info;
list_item loc;

b_heap1_node(int k, GenPtr i ) 
{ 
  key = k; info = i; loc = 0; }

  LEDA_MEMORY(b_heap1_node)

};

typedef b_heap1_node* b_heap1_item;

typedef list<b_heap1_item>* b_heap1_bucket;


class __exportC b_heap1 {

    int min;
    int max;
    int low;
    int high;
    
    int _size;
    
    array<b_heap1_bucket>  T;

		
virtual void copy_inf(GenPtr&)   const {}

virtual void clear_inf(GenPtr&)  const {}

virtual void print_inf(GenPtr)   const {}

public:

b_heap1(int a, int b);
virtual ~b_heap1() { clear(); }
b_heap1(const b_heap1& h);
b_heap1& operator=(const b_heap1& h);

b_heap1_item insert(int key, GenPtr info) ;

b_heap1_item find_min();
b_heap1_item find_max();
void del_min();
void del_max();
void decrease_key(b_heap1_item it, int k);
void increase_key(b_heap1_item it, int k);

void delete_item(b_heap1_item it);
void clear();

GenPtr inf(b_heap1_item it) { return it->info; }
int key(b_heap1_item it)  { return it->key; }
int empty()              { return (find_min()==0) ? true : false; }

int size() const {return _size;}

void print();

int lower_bound() const {return low;}
int upper_bound() const {return high;}

};

LEDA_END_NAMESPACE

#endif

