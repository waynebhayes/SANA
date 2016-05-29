/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  m_heap1.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:48 $

#ifndef LEDA_M_HEAP1_H
#define LEDA_M_HEAP1_H

//------------------------------------------------------------------------------
// m_heap1 : monotonic heaps 
//
// a) the sequence of minimum keys (over time) is monotonic (non-decreasing)
// b) the difference of minimum and maximum key is bounded by a constant M
//
// Implementation: cyclic array of lists
//
// Stefan Naeher  (1991)
//------------------------------------------------------------------------------


#include <LEDA/system/basic.h>
#include <LEDA/core/list.h>

LEDA_BEGIN_NAMESPACE


class __exportC m_heap1_elem;
class __exportC m_heap1;


typedef m_heap1_elem* m_heap1_item;

class m_heap1_elem {

   GenPtr       inf;
   m_heap1_item succ;
   m_heap1_item pred;

friend class m_heap1;

LEDA_MEMORY(m_heap1_elem)
};




class __exportC m_heap1 {

    m_heap1_item T;
    m_heap1_item T_last;
    m_heap1_item min_ptr;
    int          min_key;
    int          M;
    int          count;

virtual void copy_inf(GenPtr&)  const {}
virtual void clear_inf(GenPtr&) const {}
virtual void print_inf(GenPtr x) const { cout << x; }


void insert(m_heap1_item, m_heap1_elem&);
void remove(m_heap1_item);

public:

typedef m_heap1_item item;


m_heap1_item insert(GenPtr,GenPtr);
m_heap1_item find_min() const;
m_heap1_item first_item() const;
m_heap1_item next_item(m_heap1_item) const;

GenPtr       del_min();

void change_key(m_heap1_item,GenPtr);
void decrease_key(m_heap1_item it,GenPtr x) { change_key(it,x); }
void change_inf(m_heap1_item it, GenPtr x)  
{ clear_inf(it->inf);
  it->inf = x; 
  copy_inf(it->inf); 
};

void del_item(m_heap1_item);
void clear();

const GenPtr& inf(m_heap1_item it) const { return it->inf; }

GenPtr key(m_heap1_item) const
{ error_handler(1,"m_heap1::key not implemented");
  return 0; 
 }

int    size()   const     { return count; }
int    empty()  const     { return count==0; }

void   print() const;

 m_heap1(int M=1024);         
 m_heap1(int,int) { error_handler(1,"illegal constuctor"); }

virtual ~m_heap1() { delete T; }


// still to do: copy operations

 m_heap1& operator=(const m_heap1&) { return *this; }
 m_heap1(const m_heap1&) {}

};

LEDA_END_NAMESPACE

#endif

