/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  e_value.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:33 $

#ifndef E_VALUE_H
#define E_VALUE_H

#include <LEDA/system/event.h>

LEDA_BEGIN_NAMESPACE

template <class T, class visitor> class e_value; 

#define E_VALUE_VISITOR_TYPE(TYPE) \
  struct TYPE \
  { void operator()(self& c, const_reference x) \
    { TYPE##_event.occur(param2<self&,const_reference>(c,x)); } \
    \
    void operator()(self& c0, const_self& c1) \
    { TYPE##_event.occur(param2<self&,const_self&>(c0,c1)); } \
    \
    void operator()(const_self& c0, const_self& c1) \
    { TYPE##_event.occur(param2<const_self&,const_self&>(c0,c1)); } \
    \
    void operator()(self& c) \
    { TYPE##_event.occur(param1<self&>(c)); } \
    \
    void operator()(const_self& c) \
    { TYPE##_event.occur(param1<const_self&>(c)); } \
    \
    void operator()(int i, const_self& c) \
    { TYPE##_event.occur(param2<const_self&,int>(c,i)); } \
    \
    void operator()(int i, self& c) \
    { TYPE##_event.occur(param2<self&,int>(c,i)); } \
  }

template <class T>
struct e_value_visitor
{
  typedef e_value<T, e_value_visitor<T> >       self;
  typedef const e_value<T, e_value_visitor<T> > const_self;
  
  typedef T        value_type;
  typedef T&       reference;
  typedef const T& const_reference;
    
  static event construct_event;
  static event copy_event;
  static event destruct_event;
  static event assign_event;
  
  static event increment_event;
  static event decrement_event;

  static event cmp_le_event;
  static event cmp_leq_event;
  static event cmp_gr_event;
  static event cmp_geq_event;
  static event cmp_eq_event;
  static event cmp_neq_event;


  E_VALUE_VISITOR_TYPE(construct);
  E_VALUE_VISITOR_TYPE(copy);
  E_VALUE_VISITOR_TYPE(destruct);
  E_VALUE_VISITOR_TYPE(assign);
  
  E_VALUE_VISITOR_TYPE(increment);
  E_VALUE_VISITOR_TYPE(decrement);
  
  E_VALUE_VISITOR_TYPE(cmp_le);
  E_VALUE_VISITOR_TYPE(cmp_leq);
  E_VALUE_VISITOR_TYPE(cmp_gr);
  E_VALUE_VISITOR_TYPE(cmp_geq);
  E_VALUE_VISITOR_TYPE(cmp_eq);
  E_VALUE_VISITOR_TYPE(cmp_neq);

};

template <class T> event e_value_visitor<T>::construct_event;
template <class T> event e_value_visitor<T>::copy_event;
template <class T> event e_value_visitor<T>::destruct_event;
template <class T> event e_value_visitor<T>::assign_event;

template <class T> event e_value_visitor<T>::increment_event;
template <class T> event e_value_visitor<T>::decrement_event;

template <class T> event e_value_visitor<T>::cmp_le_event;
template <class T> event e_value_visitor<T>::cmp_leq_event;
template <class T> event e_value_visitor<T>::cmp_gr_event;
template <class T> event e_value_visitor<T>::cmp_geq_event;
template <class T> event e_value_visitor<T>::cmp_eq_event;
template <class T> event e_value_visitor<T>::cmp_neq_event;


template <class T, class visitor = e_value_visitor<T> >
class e_value : public visitor
{   
  public:
  
  typedef e_value<T,visitor>       self;
  typedef const e_value<T,visitor> const_self;

  typedef T        value_type;
  typedef T&       reference;
  typedef T*       pointer; 
  typedef const T& const_reference;

  protected:
  
  value_type val;
  
  public:
  
  const_reference data() const { return val; }

  e_value(const_reference x = T()) : val(x) 
  { typename visitor::construct V; V(*this,x); }    
  
	e_value(const_self& c) : val(c.val)
  { typename visitor::copy V; V(*this,c); }
   
  self& operator=(const_self& c)
  { typename visitor::assign V; V(*this,c); 
    val = c.val;
    return *this; 
  }  
  
  self& operator=(const_reference x) 
  { typename visitor::assign V; V(*this,x); 
    val = x; 
    return *this; 
  }    

 ~e_value() { typename visitor::destruct V; V(*this); }
 
  // --------------------------------------
  // compare operations
  // --------------------------------------
  
  bool operator<(const_self& c) const
  { typename visitor::cmp_le V; V(*this,c);
    return val < c.val; 
  }
  
  bool operator>(const_self& c) const
  { typename visitor::cmp_gr V; V(*this,c);
    return val > c.val; 
  }
  
  bool operator<=(const_self& c) const
  { typename visitor::cmp_leq V; V(*this,c);
    return val <= c.val; 
  }
    
  bool operator>=(const_self& c) const
  { typename visitor::cmp_geq V; V(*this,c);
    return val >= c.val; 
  }
      
  bool operator==(const_self& c) const
  { typename visitor::cmp_eq V; V(*this,c);
    return val == c.val; 
  }  
  
  bool operator!=(const_self& c) const
  { typename visitor::cmp_neq V; V(*this,c);
    return val != c.val; 
  }  

  // --------------------------------------
  // arithmetic operations
  // --------------------------------------
   
  self& operator++() 
  { val++;
    typename visitor::increment V; V(*this);        
    return *this;
  }
  
  self& operator--() 
  { val--;
    typename visitor::decrement V; V(*this);
    return *this;
  }

  value_type operator++(int) 
  { value_type tmp = val++;    
    typename visitor::increment V; V(*this,tmp);        
    return tmp;
  }
        
  value_type operator--(int) 
  { value_type tmp = val--;    
    typename visitor::decrement V; V(*this,tmp);        
    return tmp;
  }
      
  // --------------------------------------
  // stream operations
  // --------------------------------------
  
  friend std::ostream& operator<<(std::ostream& os, const_self& c) 
  { os << c.val;
    return os;
  }
  
  operator reference() { return val;  }
};


template <class T, class V>
bool operator<(const e_value<T,V>& c, T val) 
{ typename e_value<T,V>::cmp_le Vis; Vis(c,val);
  return c.data() < val; 
}

template <class T, class V>
bool operator>(const e_value<T,V>& c, T val) 
{ typename e_value<T,V>::cmp_gr Vis; Vis(c,val);
  return c.data() > val; 
}

template <class T, class V>
bool operator<(T val, const e_value<T,V>& c) 
{ typename e_value<T,V>::cmp_gr Vis; Vis(c,val);
  return val < c.data(); 
}

template <class T, class V>
bool operator>(T val, const e_value<T,V>& c) 
{ typename e_value<T,V>::cmp_le Vis; Vis(c,val);  
  return val > c.data(); 
}

template <class T, class V>
bool operator<=(const e_value<T,V>& c, T val) 
{ typename e_value<T,V>::cmp_leq Vis; Vis(c,val);  
  return c.data() <= val; 
}

template <class T, class V>
bool operator>=(T val, const e_value<T,V>& c) 
{ typename e_value<T,V>::cmp_leq Vis; Vis(c,val);
  return val >= c.data(); 
}

template <class T, class V>
bool operator<=(T val, const e_value<T,V>& c) 
{ typename e_value<T,V>::cmp_geq Vis; Vis(c,val);
  return val <= c.data(); 
}

template <class T, class V>
bool operator>=(const e_value<T,V>& c, T val) 
{ typename e_value<T,V>::cmp_geq Vis; Vis(c,val);
  return c.data() >= val; 
}

template <class T, class V>
bool operator==(const e_value<T,V>& c, T val) 
{ typename e_value<T,V>::cmp_eq Vis; Vis(c,val);
  return c.data() == val; 
}

template <class T, class V>
bool operator==(T val, const e_value<T,V>& c) 
{ return operator==(c,val); }
  
template <class T, class V>
bool operator!=(const e_value<T,V>& c, T val) 
{ typename e_value<T,V>::cmp_neq Vis; Vis(c,val);
  return c.data() != val; 
}

template <class T, class V>
bool operator!=(T val, const e_value<T,V>& c) 
{ return operator!=(c,val); }


LEDA_END_NAMESPACE

#endif
