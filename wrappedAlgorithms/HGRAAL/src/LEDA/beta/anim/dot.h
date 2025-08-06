/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  dot.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:33 $

#ifndef DOT_H
#define DOT_H

#include <LEDA/beta/anim/view.h>

LEDA_BEGIN_NAMESPACE

//---------------------------------------------------------------------------
// class dot
// ---------------------------------------------------------------------------

class dot : public base_element
{
  point     pos;       // dot position
   
  rectangle c_cur_pos; // current cell rectangle
  rectangle c_new_pos; // new cell rectangle
  double    c_height;  // cell height
  double    c_width;   // cell width
  color     c_col;     // cell fill color
  color     c_bcol;    // cell border color
  
  rectangle v_cur_pos; // current value rectangle
  rectangle v_new_pos; // new value rectangle  
  double    v_radius;  // dot radius
  double    v_height;  // value height
  double    v_stretch; // value stretch factor
  color     v_col;     // value fill color
  color     v_bcol;    // value border color
      
  void set_value_layout();
  void set_cell_layout();  
  void clear(rectangle);

  void attach(view&);
  void detach(view&);

  public:
  
  dot();
  dot(const dot&);
  dot& operator=(const dot&); 
 
  dot(double val, point = point(0,0));
  dot(double val, double stretch, point = point(0,0));   
 
 ~dot();
  
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
  
  double set_stretch_factor(double);
  double get_stretch_factor() const;

  color  set_value_color(color c);
  color  get_value_color() const;

  color  set_value_border_color(color c);
  color  get_value_border_color() const;

  double set_value_radius(double);
  double get_value_radius() const;
    
  double set_value(double s);
  double get_value() const;
        
  // ----------------------------------
  // animated functions  
  // ----------------------------------    
  
  void swap(dot&);      
  void swap(dot&, const path&);
  void swap(dot&, const path&, const path&);
  
  void move(point);  
  void move(point, const path&);
  
  void translate(double,double);
  void translate(double,double, const path&);
  
  void assign(double);
  void assign(dot&);
  void assign(dot&, const path&);
};

// ---------------------------------------------------------------------------
//  dot animations 
// ---------------------------------------------------------------------------

class dot_swap : public base_animation
{ 
  protected:

  dot&  D1;       // dot 1
  dot&  D2;       // dot 2
  
  view& V;        // view
  
  dot   hD1;      // helper dot 1
  dot   hD2;      // helper dot 2
  
  path  Path1;    // animation path for dot hD1
  path  Path2;    // animation path for dot hD2
  
  int   num;      // max. number of steps
  
  list<point> L1; // contains coordinates of hD1
  list<point> L2; // contains coordinates of hD2
    
  public:

  dot_swap(dot&, dot&); 
  dot_swap(dot&, dot&, const path&); 
  dot_swap(dot&, dot&, const path&, const path&); 
  
  void init(int);  
  int  steps() const { return num; }
  
  void start();  
  void step();
  void finish();
};



class dot_assign : public base_animation
{ 
  dot& D1;    // dot 1
  dot& D2;    // dot 2
  
  view&  V;   // view
  
  dot  hD1;    // helper dot

  path   Path;
  
  list<point> L;

  double val;

  int    num;
  
  public: 

  dot_assign(dot&, double x);
  dot_assign(dot&, dot&);
  dot_assign(dot&, dot&, const path&);
  
  void init(int);
  
  int steps() const { return num; }

  void start();
  void step();
  void finish();
};

LEDA_END_NAMESPACE

#endif
