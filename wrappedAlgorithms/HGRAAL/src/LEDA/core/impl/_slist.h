/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  slist.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:45 $

#ifndef LEDA_IMPL_SLIST_H
#define LEDA_IMPL_SLIST_H

//------------------------------------------------------------------------------
// simply linked lists
//------------------------------------------------------------------------------


#include <LEDA/system/basic.h>

LEDA_BEGIN_NAMESPACE


class SLIST; 
class slink;

typedef slink* slist_item;

//------------------------------------------------------------------------------
// class slink 
//------------------------------------------------------------------------------

class __exportC slink {

  friend class __exportC SLIST;

  slink* succ;
  GenPtr e;

  slink(GenPtr a, slink* suc) { e = a; succ = suc; }

  LEDA_MEMORY(slink)

};


//------------------------------------------------------------------------------
// SLIST: base class for all simply linked Lists
//------------------------------------------------------------------------------

class __exportC SLIST {

   slink* h;                     //head
   slink* t;                     //tail
   int    count;                 //length of List

virtual void clear_el(GenPtr&) const {}
virtual void copy_el(GenPtr&)  const {}
virtual int  el_type_id() const { return UNKNOWN_TYPE_ID; }

public:

   int space()  const { return sizeof(SLIST) + count * sizeof(slink); }
   int length() const { return count; }
   bool empty()  const { return (count==0);}

   slink* insert(GenPtr, slink*);


   slink* push(GenPtr a)   
   { count++;
     h = new slink(a,h); 
     if (t==0) t = h;
     return h;
   }

   slink* append(GenPtr a)
   { count++;
     if (t) t = t->succ = new slink(a,0); 
     else   t = h = new slink(a,0); 
     return t;
   } 

   slink* first()               const { return h; }
   slink* first_item()          const { return h; }
   slink* last()                const { return t; }
   slink* last_item()           const { return t; }
   slink* next_item(slink* p)   const { return p ? p->succ : 0; }
   slink* succ(slink* l)        const { return l->succ; }
   slink* cyclic_succ(slink* l) const { return l->succ? l->succ : h; }

   void conc(SLIST&);

   GenPtr& head() const 
   { if (h == nil) error_handler(1,"slist::head(): empty list"); return h->e; }

   GenPtr& tail() const
   { if (h == nil) error_handler(1,"slist::tail(): empty list"); return t->e; }

   GenPtr pop()    
   { if (h == 0) error_handler(1,"slist: pop on empty list.");
     slink* q = h; 
     GenPtr x = q->e;
     h = h->succ; 
     if (h == 0) t = 0;
     count--;
     delete q;
     return x;
   }

   void del_succ(slink* p)    
   { slink* q = p->succ;
     if (q == t) t = p;
     p->succ = q->succ; 
     delete q;
     count--;
   }

   GenPtr& entry(slink* l) const { return l->e; }

   void clear();

   SLIST();    
   SLIST(GenPtr a);
   SLIST& operator=(const SLIST&); 
   SLIST(const SLIST&);
   virtual ~SLIST()     { clear(); }

};


LEDA_END_NAMESPACE

#endif
