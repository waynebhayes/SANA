/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  gw_view.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:34 $

#include <LEDA/graphics/graphwin.h>
#include <LEDA/graphics/motion.h>

LEDA_BEGIN_NAMESPACE

class __exportC gw_element {

friend class __exportC gw_view;

protected:

  GraphWin* gwp;


  node  cell_node;
  node  value_node;
  edge  value_edge;

  double  val;
  point   pos;
  color   cell_color;
  color   value_color;
  int     width;
  int     height;

  gw_node_shape cell_shape;
  gw_node_shape value_shape;



  GraphWin* get_graphwin()   const { return gwp; }
  node      get_value_node() const { return value_node; }
  edge      get_value_edge() const { return value_edge; }
  node      get_cell_node()  const { return cell_node; }

  double    pix_to_real(double) const;

  virtual point value_position(point, double) = 0;
  virtual void  visualize_value()             = 0;
  virtual void  visualize_cell_color()        = 0;


public:

  gw_element(double, int, int, color, color, gw_node_shape,gw_node_shape);
  gw_element(const gw_element*, int, int, color, color, gw_node_shape);
  
  void attach(GraphWin& gw);
  void detach();
  bool is_attached() const;


  double set_value(double x);
  double get_value() const;

  color  set_cell_color(color col);
  color  get_cell_color() const;

  color  set_value_color(color col);
  color  get_value_color() const;

  point  set_position(point p);
  point  get_position() const;

  point  set_value_position(point p);
  point  get_value_position() const;

  point  set_position(double x, double y) { return set_position(point(x,y)); }


  // animation

  void assign(double x);
  void assign(gw_element& st, motion_base* =0);
  void swap(gw_element& st, motion_base* =0, motion_base* =0);
  void move(point p, motion_base* =0);

  void translate(double dx, double dy) { move(pos.translate(dx,dy)); }

  virtual ~gw_element() { /*( detach(); */};
};



class __exportC gw_pointer_element : public gw_element {

point value_position(point p, double) { return p; }
/*
{ double d = get_value()/2;
  if (d > 0) d += pix_to_real(4);
  else d -= pix_to_real(4);
  return p.translate(0,d);
}
*/

void visualize_value() 
{ //get_graphwin()->set_label(get_value_edge(),string("%.2f",get_value())); 
 }

void visualize_cell_color() 
{ get_graphwin()->set_color(get_cell_node(),get_cell_color()); }


public:
  gw_pointer_element(const gw_element& target) : 
        gw_element(&target,10,10,yellow,blue,rectangle_node) {}

  gw_pointer_element() : gw_element(0,10,10,yellow,blue,rectangle_node) {}
  
void setnull() { value_node = 0; }

};




class __exportC gw_stick_element : public gw_element {

point value_position(point p, double)
{ double d = get_value()/2;
  if (d >= 0) d += pix_to_real(4);
  else d -= pix_to_real(4);
  return p.translate(0,d);
}

void visualize_value() 
{ get_graphwin()->set_radius2(get_value_node(),fabs(get_value())/2); }

void visualize_cell_color() 
{ get_graphwin()->set_color(get_cell_node(),get_cell_color()); }


public:
  gw_stick_element() : 
     //gw_element(0.0,16,3,grey2,ivory,rectangle_node,rectangle_node) {}
     gw_element(0.0,24,4,grey2,ivory,rectangle_node,rectangle_node) {}

  gw_stick_element(double x, double f, point p) : 
     //gw_element(f*x,16,3,grey2,ivory,rectangle_node,rectangle_node) 
     gw_element(f*x,24,4,grey2,ivory,rectangle_node,rectangle_node) 
  { set_position(p); }
  

};

class __exportC gw_dot_element : public gw_element {


point value_position(point p, double x)
{ return p.translate(0,x); }

void visualize_value() 
{ }

void visualize_cell_color() 
{ //get_graphwin()->set_color(get_cell_node(),invisible);  
  //get_graphwin()->set_border_color(get_cell_node(),invisible);  
  //get_graphwin()->set_color(get_value_node(),get_cell_color()); 
  get_graphwin()->set_color(get_cell_node(),get_cell_color()); 
 }


public:
  gw_dot_element() : 
     gw_element(0.0,12,3,black,ivory,rectangle_node,circle_node) {}
  
void translate(double, double) {}

};



class __exportC gw_box_element : public gw_element {

point value_position(point p, double) { return p; }

void visualize_value() 
{ double x = get_value();
  if (x == int(x))
    get_graphwin()->set_label(get_value_node(),string("%d",int(x)));
  else
    get_graphwin()->set_label(get_value_node(),string("%.2f",x)); 
}

void visualize_cell_color() 
{ get_graphwin()->set_color(get_cell_node(),invisible);
  get_graphwin()->set_border_color(get_cell_node(),get_cell_color()); 
}

public:
  gw_box_element() : 
     gw_element(0.0,48,48,black,ivory,rectangle_node,rectangle_node) {}
  
void translate(double, double) {}

};


class __exportC gw_circle_element : public gw_element {

point value_position(point p, double) { return p; }

void visualize_value() 
{ double x = get_value();
  if (x == int(x))
    get_graphwin()->set_label(get_value_node(),string("%d",int(x)));
  else
    get_graphwin()->set_label(get_value_node(),string("%.2f",x)); 
}

void visualize_cell_color() 
{ get_graphwin()->set_color(get_cell_node(),invisible);
  get_graphwin()->set_border_color(get_cell_node(),get_cell_color()); 
}

public:
  gw_circle_element() : 
     gw_element(0.0,24,24,black,ivory,circle_node,circle_node) {}
  
void translate(double, double) {}

};




class __exportC gw_view
{

  GraphWin gw;

  int steps_togo;
  bool  do_anim;


public:


  gw_view(int w, int h, string label);

  void set_bg_redraw(void (*f)(window*,double,double,double,double))
  { gw.set_bg_redraw(f); }

  void set_focus_draw(void (*f)(window*,double,double,double,double))
  { gw.set_bg_node_redraw(f); }

  void win_init(double x0, double x1, double y0) 
  { gw.win_init(x0,x1,y0); 
    gw.set_node_label_font(fixed_font,24);
   }

  bool set_anim(bool b) { bool old = do_anim; do_anim = b; return old; }

  void display(int x, int y) { gw.display(x,y); }
  void display()             { gw.display(); }
  void message(string msg)   { gw.message(msg); }
  void draw()                { gw.redraw(); }
  bool wait()                { return gw.wait(); }
  void iconify()             { gw.get_window().iconify(); }


  void add_element(gw_element& st) { st.attach(gw); }
  void del_element(gw_element& st) { st.detach(); }


  void scene_begin(int steps=0);
  int  scene_end();

  void scene_step(int=1);
  void scene_play();
  void scene_finish();

  void focus(const gw_element& st) 
  { gw.clear_redraw_list();
    gw.add_redraw_list(st.get_value_node());
   }

  void unfocus() { gw.clear_redraw_list(); }

  window& get_window() const { return gw.get_window(); }


};

LEDA_END_NAMESPACE

