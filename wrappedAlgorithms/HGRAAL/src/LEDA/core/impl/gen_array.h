/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  gen_array.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:47 $

#ifndef LEDA_GEN_ARRAY_H
#define LEDA_GEN_ARRAY_H

//------------------------------------------------------------------------------
// arrays
//------------------------------------------------------------------------------

#include <LEDA/system/basic.h>

LEDA_BEGIN_NAMESPACE


class __exportC gen_array {

friend class __exportC gen_array2;

protected:
	GenPtr* v;
	GenPtr* last;
	int     sz;	
        int     Low;
        int     High;
        bool    def_cmp;

virtual int  el_type_id() const { return UNKNOWN_TYPE_ID; }

virtual int  cmp(GenPtr, GenPtr)  const { return 0; }
virtual void print_el(GenPtr&,ostream&) const {}
virtual void read_el(GenPtr& ,istream&) const {}
virtual void clear_entry(GenPtr&) const {}
virtual void copy_entry(GenPtr&)  const {}
virtual void init_entry(GenPtr&)  const {}

  void gen_quick_sort(GenPtr*,GenPtr*);
  void gen_insertion_sort(GenPtr*,GenPtr*,GenPtr*);

  int  gen_binary_search(GenPtr) const;
  int  int_binary_search(GenPtr) const;
  int  double_binary_search(GenPtr) const;

  int  gen_binary_locate(GenPtr) const;
  int  int_binary_locate(GenPtr) const;
  int  double_binary_locate(GenPtr) const;

protected:

  void sort(int l ,int h, int d); 
  void int_sort(int l, int h);

  int  binary_search(GenPtr) const;
  // returns an i such that A[i] = x or Low-1 if x not present 

  int  binary_locate(GenPtr) const;
  // locates predecessor, i.e. computes maximal i such 
  // that A[i] <= x  (Low-1 if x < A[low])

  void init();
  void init(GenPtr);
  void init(GenPtr,GenPtr);
  void init(GenPtr,GenPtr,GenPtr);
  void init(GenPtr,GenPtr,GenPtr,GenPtr);

  void clear();

public:

   gen_array();
   gen_array(int);
   gen_array(int, int);
   gen_array(const gen_array&);
   virtual ~gen_array();

   gen_array& operator=(const gen_array&);

   void    resize(int a, int b);
   void    resize(int n) { resize(0,n-1); }

   int     size() const     { return sz; }
   int     low()  const     { return Low; }
   int     high() const     { return High; }

   GenPtr& elem(int i)       { return v[i]; }
   GenPtr  elem(int i) const { return v[i]; }

   GenPtr& entry(int i) const
   { if (i<Low || i>High)
     error_handler(2,"array::entry index out of range");
     return v[i-Low];
    }

   GenPtr  inf(int i) const
   { if (i<Low || i>High)
     error_handler(2,"array::inf index out of range");
     return v[i-Low];
    }

   void set(int i, const GenPtr &e)
   { if (i<Low || i>High)
     error_handler(2,"array::entry index out of range");
     v[i] = e;
   }

   void swap(int,int);

   void permute(int,int);
   void permute();

   void print(ostream& out, string s, char space) const;
   void print(ostream& out,char space=' ') const;
   void print(string s, char space=' ')    const { print(cout,s,space);  }
   void print(char space=' ')              const { print(cout,space); }   

   void read(istream&,string);  
   void read(istream& in)      { read(in,"");  }
   void read(string s )        { read(cin,s);  }   
   void read()                 { read(cin,""); }   

// Iteration

   GenPtr first_item() const { return v; }
   GenPtr last_item()  const { return last; }
   GenPtr next_item(GenPtr p) const { return (p && p!=last) ? (GenPtr*)p+1 : 0;}
   GenPtr pred_item(GenPtr p) const { return (p && p!=v)    ? (GenPtr*)p-1 : 0;}

   GenPtr stl_next_item(GenPtr it) const { return (GenPtr*)it+1;   }
   GenPtr stl_pred_item(GenPtr it) const { return (GenPtr*)it-1;   }

};


/*------------------------------------------------------------------------*/
/* 2 dimensional arrays                                                   */
/*------------------------------------------------------------------------*/


class __exportC gen_array2 {
gen_array A;
int Low1, Low2, High1, High2;
virtual void clear_entry(GenPtr& x) const { x = 0; }
virtual void copy_entry(GenPtr& x)  const { x = 0; }
virtual void init_entry(GenPtr& x)  const { x = 0; }

protected:
void clear();
gen_array* row(int i) const { return (gen_array*)A.inf(i); }

void copy_row(gen_array*, gen_array*) const;

public:
void init(int,int,int,int);
int low1()  const { return Low1; }
int low2()  const { return Low2; }
int high1() const { return High1; }
int high2() const { return High2; }
gen_array2(int,int,int,int);
gen_array2(int,int);
virtual ~gen_array2();

gen_array2(const gen_array2& a);

gen_array2& operator=(const gen_array2& a);

};

LEDA_END_NAMESPACE

#endif
