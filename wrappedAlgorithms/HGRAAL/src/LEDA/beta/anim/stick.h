/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  stick.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:34 $

#ifndef STICK_H
#define STICK_H

#include <LEDA/beta/anim/view.h>

LEDA_BEGIN_NAMESPACE

//---------------------------------------------------------------------------
// class stick 
// ---------------------------------------------------------------------------

class stick : public base_element
{ 
  protected:
  
  point     position;   // stick position
  
  rectangle c_cur_pos;  // current cell rectangle
  rectangle c_new_pos;  // new cell rectangle
  double    c_height;   // cell height
  double    c_width;    // cell width
  color     c_col;      // cell fill color
  color     c_bcol;     // cell border color
  
  rectangle v_cur_pos;  // current value rectangle
  rectangle v_new_pos;  // new value rectangle
  double    v_height;   // value height
  double    v_stretch;  // stretch factor
  double    v_width;    // value width
  double    v_yoff;     // distance between cell and value retangle
  color     v_col;      // value fill color
  color     v_bcol;     // value border color
  
  void set_default_attr();    
  void set_value_layout(); 
  void set_cell_layout(); 
  
  void clear(rectangle);

  void attach(view&);
  void detach(view&);

  public:
  
  stick(point pos = point(0,0)); 
  stick(double value, point pos = point(0,0));
  stick(double value, double stretch, point pos = point(0,0));

  stick(const stick&);
  stick& operator=(const stick&);   

 ~stick();
  
  void redraw();

  void draw();
  void clear();
  
  rectangle get_bounding_box();

  bool intersect(segment, list<point>&);

  point  set_position(double x, double y);   
  point  set_position(point);
  point  get_position() const;
  
  color  set_cell_color(color c);
  color  get_cell_color() const;
    
  color  set_cell_border_color(color c);
  color  get_cell_border_color() const;

  double set_cell_height(double h);
  double get_cell_height() const;
  
  double set_cell_width(double w); 
  double get_cell_width() const;    
    
  color  set_value_color(color c);
  color  get_value_color() const;

  color  set_value_border_color(color c);
  color  get_value_border_color() const;

  double set_value(double);
  double get_value() const;
    
  double set_stretch_factor(double);
  double get_stretch_factor() const;
  
  double set_value_width(double w);  
  double get_value_width() const;
    
  double set_value_yoffset(double s);
  double get_value_yoffset() const;

  // ----------------------------------
  // animated member functions  
  // ----------------------------------    
  
  void swap(stick&);      
  void swap(stick&, const path&);
  void swap(stick&, const path&, const path&);
  
  void move(point);  
  void move(point, const path&);
  
  void translate(double,double);
  void translate(double,double, const path&);
  
  void assign(double);
  void assign(stick&);
  void assign(stick&, const path&);
};


// ---------------------------------------------------------------------------
//  stick animations 
// ---------------------------------------------------------------------------

class stick_swap : public base_animation
{ 
  protected:

  stick& S1;      // stick 1
  stick& S2;      // stick 2
  
  view&  V;       // view
  
  stick  hS1;     // helper stick 1
  stick  hS2;     // helper stick 2
  
  path   Path1;   // animation path for stick hS1
  path   Path2;   // animation path for stick hS2
  
  int    num;     // max. number of required steps
  
  list<point> L1; // contains coordinates of hS1
  list<point> L2; // contains coordinates of hS2
    
  public:

  stick_swap(stick& s1, stick &s2); 
  stick_swap(stick& s1, stick &s2, const path& P); 
  stick_swap(stick& s1, stick &s2, const path& P1, const path& P2); 
  
  void init(int);  
  int  steps() const { return num; }
  
  void start();  
  void step();
  void finish();
};



class stick_assign : public base_animation
{ 
  stick& S1;    // stick 1
  stick& S2;    // stick 2
  
  view&  V;     // view
  
  stick  hS1;   // helper stick

  path   Path;
  
  list<point> L;

  color  col1;
  color  col2;

  double val;

  int    num;
  
  public: 

  stick_assign(stick& s, double x);
  stick_assign(stick& s1, stick& s2);
  stick_assign(stick& s1, stick& s2, const path&);
  
  void init(int);
  
  int steps() const { return num; }

  void start();
  void step();
  void finish();
};

LEDA_END_NAMESPACE

#endif
