#ifndef CELL_H
#define CELL_H

#include <LEDA/system/event.h>

#ifdef LEDA_NAMESPACE
LEDA_BEGIN_NAMESPACE
#endif

template <class T, class visitor> class cell; 


#define CELL_VISITOR_TYPE(TYPE) \
  struct TYPE \
  { void operator()(const_reference x, self& c) \
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
    void operator()(const_self& c, int i) \
    { TYPE##_event.occur(param2<const_self&,int>(c,i)); } \
    \
    void operator()(self& c, int i) \
    { TYPE##_event.occur(param2<self&,int>(c,i)); } \
  }; 

template <class T>
struct cell_visitor
{
  typedef cell<T, cell_visitor<T> >       self;
  typedef const cell<T, cell_visitor<T> > const_self;
  
  typedef T        value_type;
  typedef T&       reference;
  typedef const T& const_reference;
    
  static event constructor_event;
  static event destructor_event;
  static event assignment_event;
  
  static event increment_event;
  static event decrement_event;

  static event lesser_event;

  CELL_VISITOR_TYPE(constructor);
  CELL_VISITOR_TYPE(destructor);
  CELL_VISITOR_TYPE(assignment);
  
  CELL_VISITOR_TYPE(increment);
  CELL_VISITOR_TYPE(decrement);
  
  CELL_VISITOR_TYPE(lesser);
};

template <class T> event cell_visitor<T>::constructor_event;
template <class T> event cell_visitor<T>::destructor_event;
template <class T> event cell_visitor<T>::assignment_event;

template <class T> event cell_visitor<T>::increment_event;
template <class T> event cell_visitor<T>::decrement_event;

template <class T> event cell_visitor<T>::lesser_event;


template <class T, class visitor = cell_visitor<T> >
class cell : public visitor
{   
  public:
  
  typedef cell<T,visitor>       self;
  typedef const cell<T,visitor> const_self;

  typedef T        value_type;
  typedef T&       reference;
  typedef T*       pointer; 
  typedef const T& const_reference;

  protected:
  
  value_type val;
  
  public:
  
  const_reference data() const { return val; }

  cell(const_reference x = T()) : val(x) 
  { typename visitor::constructor V; V(x,*this); }    
  
	cell(const_self& c) : val(c.val)
  { typename visitor::constructor V; V(*this,c); }
   
  self& operator=(const_self& c)
  { typename visitor::assignment V; V(*this,c); 
    val = c.val;
    return *this; 
  }  

  self& operator=(const_reference x) 
  { typename visitor::assignment V; V(x,*this); 
    val = x; 
    return *this; 
  }    

 ~cell() { typename visitor::destructor V; V(*this); }
 
  // --------------------------------------
  // compare operations
  // --------------------------------------
  
  bool operator<(const_self c) const
  { typename visitor::lesser V; V(*this,c);
    return val < c.val; 
  }

  friend bool operator<(const_self& c, const_reference v) 
  { typename visitor::lesser V; V(c,v);
    return c.data() < v; 
  }

  friend bool operator<(const_reference v, const_self& c) 
  { return v < c.data(); }

  
  bool operator>(const_self& c) const
  { //typename handler::greater H(*this,c);
    return val > c.val; 
  }

  friend bool operator>(const_self& c, const_reference v) 
  { return c.data() > v; }

  friend bool operator>(const_reference v, const_self& c) 
  { return v > c.data(); }

  
  bool operator<=(const_self c) const
  { //typename handler::lower H(*this,c);
    return val <= c.val; 
  }
    
  friend bool operator<=(const_self& c, const_reference v) 
  { return c.data() <= v; }

  friend bool operator<=(const_reference v, const_self& c) 
  { return v <= c.data(); }

  bool operator>=(const_self c) const
  { //typename handler::greater H(*this,c);
    return val >= c.val; 
  }

  friend bool operator>=(const_self& c, const_reference v) 
  { return c.data() >= v; }

  friend bool operator>=(const_reference v, const_self& c) 
  { return v >= c.data(); }
    
  bool operator==(const_self c) const
  { //typename handler::equal H(*this,c);
    return val == c.val; 
  }  
  
  friend bool operator==(const_self& c, const_reference v) 
  { return c.data() == v; }

  friend bool operator==(const_reference v, const_self& c) 
  { return v == c.data(); }

  bool operator!=(const_self c) const
  { //typename handler::not_equal H(*this,c);
    return val != c.val; 
  }

  friend bool operator!=(const_self& c, const_reference v) 
  { return c.data() != v; }

  friend bool operator!=(const_reference v, const_self& c) 
  { return v != c.data(); }

  // --------------------------------------
  // arithmetic operations
  // --------------------------------------
   
  self& operator++() 
  { val++;
    typename visitor::increment V; V(*this);        
    return *this;
  }
  
  self operator++(int) 
  { //typename handler::post_increment H(*this);
    return val++;
  }
  
  self& operator--() 
  { val--;
    typename visitor::decrement V; V(*this);
    return *this;
  }
      
  self operator--(int) 
  { //typename handler::post_decrement H(*this);
    value_type tmp = val;
    val--;
    return tmp;
  }  

  friend self operator-(const_self& c, const_reference v) 
  { return c.data() - v; }

  friend self operator-(const_reference v, const_self& c) 
  { return v - c.data(); }
 
  friend self operator+(const_self& c, const_reference v) 
  { return c.data() + v; }

  friend self operator+(const_reference v, const_self& c) 
  { return v + c.data(); }
  
  // --------------------------------------
  // stream operations
  // --------------------------------------
  
  friend std::ostream& operator<<(std::ostream& os, const_self& c) 
  { os << c.val;
    return os;
  }
  
  operator reference() { return val;  }
};

#ifdef LEDA_NAMESPACE
LEDA_END_NAMESPACE
#endif

#endif
