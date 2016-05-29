/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  sweep.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:45:02 $

#ifndef LEDA_SWEEP_H
#define LEDA_SWEEP_H

#include <LEDA/core/sortseq.h>

LEDA_BEGIN_NAMESPACE

template <class K, class I, class pos_type>

class sweep_line : public sortseq<K,I>
{
  pos_type  sw_pos;

  int cmp(GenPtr x, GenPtr y) const
  { return compare(sw_pos,LEDA_CONST_ACCESS(K,x),LEDA_CONST_ACCESS(K,y)); }
  
public:

  pos_type  get_position() { return sw_pos; }

  pos_type  set_position(const pos_type& new_pos) 
  { pos_type old_pos = sw_pos; sw_pos = new_pos; return old_pos; }

  pos_type  move_to(const pos_type& new_pos) { return set_position(new_pos); }

  sweep_line() {}
  sweep_line(const pos_type p) : sw_pos(p) {}
 ~sweep_line() {}

};


LEDA_END_NAMESPACE

#endif
