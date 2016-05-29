#ifndef SMART_POINTER_H
#define SMART_POINTER_H

#include <iterator>

// -----------------------------------------------------------------------------
// VC++7 iterator "specializations"
// -----------------------------------------------------------------------------

namespace std {

#if defined(_MSC_VER)
template <class _Iter> 
inline typename iterator_traits<_Iter>::difference_type* _Dist_type(const _Iter&)
{ return static_cast<typename iterator_traits<_Iter>::difference_type*>(0); }

template <class _Iter> 
inline typename iterator_traits<_Iter>::value_type* _Val_type(const _Iter&)
{ return static_cast<typename iterator_traits<_Iter>::value_type*>(0); } 

template <class _Iter> 
inline typename iterator_traits<_Iter>::iterator_category _Iter_cat(const _Iter&)
{ return iterator_traits<_Iter>::iterator_category(); }
#else


template <class _Iter> 
inline ptrdiff_t* __distance_type(const _Iter&) { return (ptrdiff_t*)0; }

template <class _Iter> 
inline typename _Iter::pointer __value_type(const _Iter&) 
{ return (typename _Iter::pointer)0; }

/*
template <class _Iter> 
inline typename iterator_traits<_Iter>::iterator_category _Iter_cat(const _Iter&)
{ return iterator_traits<_Iter>::iterator_category(); }
*/

#endif

} // end namespace std


#include <LEDA/system/event.h>

LEDA_BEGIN_NAMESPACE

template <class T, class V> class smart_pointer;

#define SP_VISITOR_TYPE(TYPE) \
  struct TYPE \
  { void operator()(self& c0, const_self& c1) \
    { TYPE##_event.occur(param2<self&,const_self&>(c0,c1)); } \
    \
    void operator()(self& c, pointer p) \
    { TYPE##_event.occur(param2<self&,pointer>(c,p)); } \
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
struct sp_visitor
{
  typedef smart_pointer<T, sp_visitor<T> >       self;
  typedef const smart_pointer<T, sp_visitor<T> > const_self;
  
  typedef T* pointer;
   
  static event constructor_event;
  static event destructor_event;
  static event assignment_event;
  
  static event access_event;
  
  static event decrement_event;
  static event increment_event;
  
  static event lesser_event;
  static event lesser_equal_event;
  static event greater_event;
  static event greater_equal_event;
  static event equal_event;
  static event not_equal_event;
	  	
  SP_VISITOR_TYPE(constructor);
  SP_VISITOR_TYPE(destructor);
  SP_VISITOR_TYPE(assignment);	
  SP_VISITOR_TYPE(access);

  SP_VISITOR_TYPE(lesser);
  SP_VISITOR_TYPE(lesser_equal);
  SP_VISITOR_TYPE(greater);
  SP_VISITOR_TYPE(greater_equal);
  SP_VISITOR_TYPE(not_equal);
  SP_VISITOR_TYPE(equal);
  
  SP_VISITOR_TYPE(increment);
  SP_VISITOR_TYPE(decrement);
};

template <class T> event sp_visitor<T>::constructor_event;
template <class T> event sp_visitor<T>::destructor_event;
template <class T> event sp_visitor<T>::assignment_event;

template <class T> event sp_visitor<T>::access_event;

template <class T> event sp_visitor<T>::increment_event;
template <class T> event sp_visitor<T>::decrement_event;

template <class T> event sp_visitor<T>::lesser_event;
template <class T> event sp_visitor<T>::lesser_equal_event;
template <class T> event sp_visitor<T>::greater_event;
template <class T> event sp_visitor<T>::greater_equal_event;
template <class T> event sp_visitor<T>::equal_event;
template <class T> event sp_visitor<T>::not_equal_event;




template <class T, class visitor = sp_visitor<T> > 
class smart_pointer : public visitor
{  
  T* p; 
  
  public:
     
  typedef std::random_access_iterator_tag iterator_category;
  typedef ptrdiff_t difference_type;    
  
  typedef T         value_type;
  typedef T&        reference;
  typedef const T&  const_reference; 
  typedef T*        pointer;
  
  typedef smart_pointer<T,visitor>       self;
  typedef const smart_pointer<T,visitor> const_self;
  
  pointer data() const { return p; }
  
  smart_pointer(pointer q = 0) : p(q)
  { typename visitor::constructor V; V(*this,q); }
    
  smart_pointer(const_self& sp) : p(sp.p)
  { typename visitor::constructor V; V(*this,sp); }

 ~smart_pointer() 
  { typename visitor::destructor V; V(*this); }
  
  self& operator=(const_self& sp)
  { typename visitor::assignment V; V(*this,sp); 
    p = sp.p;
    return *this;
  }
  
  self& operator=(pointer q)
  { typename visitor::assignment V; V(*this,x); 
    p = q;
    return *this;
  }  
  
  reference operator[](int x)  
  { typename visitor::access V; V(*this,x);
    return p[x]; 
  }  
 
  const_reference operator[](int x) const
  { typename visitor::access V; V(*this,x);
    return p[x]; 
  }
  
  reference operator*() 
  { typename visitor::access V; V(*this);
    return *p; 
  }
  
  const_reference operator*() const 
  { typename visitor::access V; V(*this);
    return *p; 
  }
  
  pointer operator->() const 
  { typename visitor::access V; V(*this);
    return p; 
  }
      
  operator pointer() { return p;  }  
  
  // --------------------------------------
  // arithmetic operations
  // --------------------------------------

  self& operator++() 
  { p++; 
    typename visitor::increment V; V(*this);        
    return *this; 
  }    
  
  self& operator--() 
  { p--;        
    typename visitor::decrement V; V(*this);        
    return *this; 
  }
  
  self operator++(int) 
  { T* tmp = p++;    
    typename visitor::increment V; V(*this,tmp); 
    return tmp; 
  }
      
  self operator--(int) 
  { T* tmp = p--; 
    typename visitor::decrement V; V(*this,tmp);     
    return tmp; 
  }
  
  self& operator+=(int i)
  { p + i; 
    typename visitor::increment V; V(*this,i);    
    return *this;  
  }
  
  self& operator-=(int i)
  { p - i; 
    typename visitor::decrement V; V(*this,i);    
    return *this;  
  }
  
  self operator+(int i) const { return p + i; } 
  self operator-(int i) const { return p - i; }
  
  int operator-(const_self& c) const { return p - c.p; }
  
  // --------------------------------------
  // compare operations
  // --------------------------------------
  
  bool operator<(const_self& c) const
  { typename visitor::lesser V; V(*this,c);
    return p < c.p; 
  }
  
  bool operator>(const_self& c) const
  { typename visitor::greater V; V(*this,c);
    return p > c.p; 
  }
  
  bool operator<=(const_self& c) const
  { typename visitor::lesser_equal V; V(*this,c);
    return p <= c.p; 
  }
    
  bool operator>=(const_self& c) const
  { typename visitor::greater_equal V; V(*this,c);
    return p >= c.p; 
  }
      
  bool operator==(const_self& c) const
  { typename visitor::equal V; V(*this,c);
    return p == c.p; 
  }  
  
  bool operator!=(const_self& c) const
  { typename visitor::not_equal V; V(*this,c);
    return p != c.p; 
  }  
};

template <class T, class V>
bool operator<(const smart_pointer<T,V>& c, T* p) 
{ typename smart_pointer<T,V>::lesser Vis; Vis(c,p);
  return c.data() < p; 
}

template <class T, class V>
bool operator>(const smart_pointer<T,V>& c, T* p) 
{ typename smart_pointer<T,V>::greater Vis; Vis(c,p);
  return c.data() > p; 
}

template <class T, class V>
bool operator<(T* p, const smart_pointer<T,V>& c) 
{ typename smart_pointer<T,V>::greater Vis; Vis(c,p);
  return p < c.data(); 
}

template <class T, class V>
bool operator>(T* p, const smart_pointer<T,V>& c) 
{ typename smart_pointer<T,V>::lesser Vis; Vis(c,p);  
  return p > c.data(); 
}

template <class T, class V>
bool operator<=(const smart_pointer<T,V>& c, T* p) 
{ typename smart_pointer<T,V>::lesser_equal Vis; Vis(c,p);  
  return c.data() <= p; 
}

template <class T, class V>
bool operator>=(T* p, const smart_pointer<T,V>& c) 
{ typename smart_pointer<T,V>::lesser_equal Vis; Vis(c,p);
  return p >= c.data(); 
}

template <class T, class V>
bool operator<=(T* p, const smart_pointer<T,V>& c) 
{ typename smart_pointer<T,V>::greater_equal Vis; Vis(c,p);
  return p <= c.data(); 
}

template <class T, class V>
bool operator>=(const smart_pointer<T,V>& c, T* p) 
{ typename smart_pointer<T,V>::greater_equal Vis; Vis(c,p);
  return c.data() >= p; 
}

template <class T, class V>
bool operator==(const smart_pointer<T,V>& c, T* p) 
{ typename smart_pointer<T,V>::equal Vis; Vis(c,p);
  return c.data() == p; 
}

template <class T, class V>
bool operator==(T* p, const smart_pointer<T,V>& c) 
{ return operator==(c,p); }
  
template <class T, class V>
bool operator!=(const smart_pointer<T,V>& c, T* p) 
{ typename smart_pointer<T,V>::not_equal Vis; Vis(c,p);
  return c.data() != p; 
}

template <class T, class V>
bool operator!=(T* p, const smart_pointer<T,V>& c) 
{ return operator!=(c,p); }

LEDA_END_NAMESPACE

#endif
