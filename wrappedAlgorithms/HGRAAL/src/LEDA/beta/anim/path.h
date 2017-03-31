/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  path.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:34 $

#ifndef PATH_H
#define PATH_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 430256
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/core/list.h>
#include <LEDA/geo/line.h>

LEDA_BEGIN_NAMESPACE

enum coord_t { ABSOLUTE, RELATIVE, UNDEFINED};
  
class base_coord
{ 
  protected:
  
  coord_t Type;    
  double  Coord;   
  
  public:  
  
  base_coord() : Type(UNDEFINED), Coord(0) {}  
  
  double  coord() const { return Coord; }   
  coord_t type()  const { return Type; }
};


struct abs_coord : public base_coord {   
  abs_coord(double x) { Type = ABSOLUTE; Coord = x; }
};

struct rel_coord : public base_coord { 
  rel_coord(double x) { Type = RELATIVE; Coord = x; }
};


class path_point 
{
  protected:
  
  base_coord x;
  base_coord y;
  
  public:
  
  path_point() {}
  
  template <class XCoord, class YCoord>
  path_point(XCoord X, YCoord Y) : x(X), y(Y) {}

  double xcoord() const { return x.coord(); }
  double ycoord() const { return y.coord(); }
  
  coord_t xcoord_type() const { return x.type(); }
  coord_t ycoord_type() const { return y.type(); }
};


enum trans_t { linear, circular };

/*{\Manpage {path} {} {Path}}*/

class __exportC path
{
/*{\Mdefinition
    The data type |path| defines an abstract path in the plane.
    There are two possibilities to define a path:
}*/
  
  protected:
    
  list<path_point> Path;
  
  double  angle;
  trans_t type;

  public:
  
/*{\Mcreation P}*/
  path() : angle(0), type(linear) 
/*{\Mcreate creates an instance |\Mvar| of type |\Mname| and
            initializes it to the empty path. }*/
  {}
  
/*{\Moperations 1.7 2.3 }*/

  trans_t set_transition_type(trans_t t);
/*{\Mopl sets the transition type to |t| and 
         returns the previous value.}*/

  trans_t get_transition_type() const;
/*{\Mopl returns the transition type.}*/
    
  template <class XCoord, class YCoord>
  void append(XCoord x, YCoord y) 
/*{\Mopl appends the abstract point |(x,y)| to the path and 
         sets the transition type to |linear|.}*/  
  { type = linear;
    Path.append(path_point(x,y)); 
  }
  
  void clear();
/*{\Mopl removes all abstract points in |P|.}*/
    
  bool empty() const; 
/*{\Mopl returns true if |P| stores no abstract points, otherwise false.}*/

  int  size() const;
/*{\Mopl returns the number of points stored in |P|.}*/
    
  double set_angle(double alpha);
/*{\Mopl sets the angle to |alpha| and returns the previous value. 
         The transition type is now |circular|. }*/
  
  double get_angle() const;
/*{\Mopl returns the angle.}*/
    
  double length(point p0, point p1);
/*{\Mopl returns the length of the path between |p0| and |p1|. }*/
  
  list<point> transition(point p0, point p1, int n);
/*{\Mopl computes a transition between |p0| and |p1| with |n| points.}*/
  
  // ------------------------------------------------------
  // iteration
  // ------------------------------------------------------

  typedef list_item item;  
  
  const path_point& inf(list_item x) const { return Path[x]; }  
  
  item first_item()           const { return Path.first_item(); }
  item next_item(list_item x) const { return Path.next_item(x); }
  item last_item()            const { return Path.last_item();  }   
};

/*{\Mtext
\bigskip
{\bf Non-Member Functions} 
}*/

list<point> linear_transition(path& Path, point p0, point p1, int n);
/*{\Mfunc    computes a linear transition between |p0| and |p1| 
             with |n| points by using the path |Path|.}*/ 

list<point> circular_transition(point p0, point p1, double alpha, int n);
/*{\Mfunc    computes a circular transition between |p0| and |p1| 
             with |n| points by using the angle |alpha|.}*/ 



/*{\Mexample  
\begin{verbatim}
#include <LEDA/beta/anim/path.h>

#ifdef LEDA_NAMESPACE
using namespace leda;
#endif

int main()
{ path P;   
  P.append(rel_coord(0.0), abs_coord(20));
  P.append(rel_coord(1.0), abs_coord(20));
  
  bool circular = false;
  int  angle = 45, steps = 100;
  
  window W;
  W.button("Exit",1,exit);
  W.bool_item("Circular", circular);
  W.int_item("Angle", angle, 0, 360);
  W.int_item("Steps", steps, 1, 1000);
  W.display(window::center,window::center);
      
  while (1) 
  { point p, p0, p1;      
    W >> p0; W << p0; W >> p1; W << p1;     
  
    list<point> L;      
    if (circular == true) 
      L = circular_transition(p0,p1,angle,steps);
    else
      L = linear_transition(P,p0,p1,steps);    
  
    forall(p,L) W << p;            
  
    if (W.read_mouse() == 1) break;      
    W.clear();            
  } 
  return 0;
}
\end{verbatim}
}*/

LEDA_END_NAMESPACE

#if LEDA_ROOT_INCL_ID == 430256
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

#endif

