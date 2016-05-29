
#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 430999
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/system/basic.h>
#include <LEDA/system/stream.h>

LEDA_BEGIN_NAMESPACE


template<class T> 
class cell 
{
  T val;
  unsigned id;

  static unsigned count;

public:

  unsigned get_id() { return id; }

  cell() { 
    id = count++;
    cout << string("default cell: %d",id) << endl;
  }

  cell(const T& x) : val(x) {
    id = count++;
    cout << string("init cell: %d  val = %d",id,x) << endl;
  }

  cell(const cell<T>& x) : val(x.val) {
    id = count++;
    cout << string("copy cell: %d  src = %d",id,x.id) << endl;
  }

  cell<T>& operator=(const cell<T>& x)  { 
    cout << string("assign cell: %d  src = %d",id,x.id) << endl;
    val = x.val; 
    return *this; 
  }
  
  bool operator< (const cell<T>& x) const { return val <  x.val; }
  bool operator> (const cell<T>& x) const { return val >  x.val; }
  bool operator<=(const cell<T>& x) const { return val <= x.val; }
  bool operator>=(const cell<T>& x) const { return val >= x.val; }
  bool operator==(const cell<T>& x) const { return val == x.val; }
  bool operator!=(const cell<T>& x) const { return val != x.val; }
  
  friend ostream& operator<<(ostream& os, const cell<T>& x) 
  { return os << x.val; }

  friend istream& operator>>(istream& is, cell<T>& x) 
  { return is >> x.val; }

};


template<class T>
class v_iterator {

 typedef v_iterator<T> self;

 typedef cell<T>  value_type;
 typedef cell<T>* pointer;
 typedef cell<T>& reference;

/*
 typedef const T* const_pointer;
 typedef const T& const_reference;
*/

 typedef std::bidirectional_iterator_tag iterator_category;
 typedef std::ptrdiff_t                  difference_type;


 pointer ptr;
 unsigned id;

 static unsigned count;

public:

 v_iterator()     : ptr(0)  {
    id = count++;
    cout << string("default iterator: %d",id) << endl;
 }

 v_iterator(pointer q) : ptr(q)  {
    cout << string("init iterator: %d  ptr = %d",q->get_id()) << endl;
 }


  v_iterator(const self& x) : ptr(x.ptr) {
    id = count++;
    cout << string("copy iterator: %d  src = %d",id,x.id) << endl;
  }

  self& operator=(const self& x)  { 
    cout << string("assign iterator: %d  src = %d",id,x.id) << endl;
    ptr = x.ptr; 
    return *this; 
  }

 self& operator++() { 
   cout << string("increment iterator: %d ++",id) << endl;
   ptr++; return *this; 
 }

 self& operator--() { 
   cout << string("decrement iterator: %d --",id) << endl;
   ptr--; return *this; 
 }

 self  operator++(int) { self tmp = *this; ++(*this); return tmp; }
 self  operator--(int) { self tmp = *this; --(*this); return tmp; }

 self  operator+(ptrdiff_t d) const { 
   cout << string("increment iterator: %d + %d",id,d) << endl;
   return self(ptr+d); 
 } 

 self  operator-(ptrdiff_t d) const { 
   cout << string("decrement iterator: %d - %d",id,d) << endl;
   return self(ptr-d); 
 } 


 ptrdiff_t operator-(const self& it) const { 
   cout << string("iterator diff: %d - %d",id,it.id) << endl;
  return ptr - it.ptr; 
 }


 reference operator*()  const { return *ptr; }
 pointer   operator->() const { return  ptr; }

 bool operator< (const self& it) const { return ptr <  it.ptr; }
 bool operator<=(const self& it) const { return ptr <= it.ptr; }
 bool operator> (const self& it) const { return ptr >  it.ptr; }
 bool operator>=(const self& it) const { return ptr >= it.ptr; }
 bool operator==(const self& it) const { return ptr == it.ptr; }
 bool operator!=(const self& it) const { return ptr != it.ptr; }

};


/*
namespace std {

template<class T>
inline ptrdiff_t* _Dist_type(const v_iterator<T>&) { return (ptrdiff_t*)0; }

template<class T>
inline ptrdiff_t* _Val_type(const v_iterator<T>&)  { return (T*)0; }

template<class T>
inline random_access_iterator_tag _Iter_cat(const v_iterator<T>&)  
{ random_access_iterator_tag cat;
  return cat; 
 }

}
*/



template <class T>
class v_vector {

cell<T>* mem;
cell<T>* vec;

unsigned sz;

public:

v_vector(unsigned n) {
   mem = new cell<T>[n+2];
   vec = mem+1;
   sz = n;
}

~v_vector() { delete[] mem; }


cell<T>&       operator[](int i)       { return vec[i]; }
const cell<T>& operator[](int i) const { return vec[i]; }


v_iterator<T>  begin() { return vec;    }
v_iterator<T>  end()   { return vec+sz; }

};





LEDA_END_NAMESPACE

#if LEDA_ROOT_INCL_ID == 430999
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif


