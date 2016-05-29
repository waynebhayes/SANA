/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  view_base.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.4 $  $Date: 2005/04/14 10:44:34 $

#ifndef VIEW_BASE_TYPES_H
#define VIEW_BASE_TYPES_H

#include <LEDA/core/map.h>
#include <LEDA/graphics/window.h>
#include <LEDA/graphics/motion.h>

LEDA_BEGIN_NAMESPACE

class base_element;
struct base_animation;
struct base_action;

typedef base_element* elem_item;
typedef base_animation* anim_item;
typedef base_action* action_item;

//---------------------------------------------------------------------------
// class view
// ---------------------------------------------------------------------------

/*{\Manpage {view} {} {View}}*/

/*{\Mdefinition
}*/

class view 
{ protected:

  struct redraw_handler : public window_handler
  { view& V;    
    redraw_handler(view& v) : V(v) {}    
    
    void operator()(); 
  };    
 
  redraw_handler redraw_h;
  
  friend struct view::redraw_handler;

  window W;
  
  list<elem_item> Elems;
  list<anim_item> Anims;
  
  map<window*,list<action_item> >Action;  
  
  action_item Menu;
  action_item StdAction;

  int    max_steps;
  bool   first_step;  
  double scale_factor;
  
  int    sleep_time;  // time in msec
  double listen_time;
  
  bool listen();  
  void set_listen_time();
  
  public: 
 
  view(string label = "View");
  view(int w, int h, string label = "View");
 ~view(); 
    
  void draw() { W.redraw(); }
  
  void win_init(double x0, double x1, double y0);
  
  double  get_scale_factor() const;  
  window& get_window() { return W; } 

  void display();
  void display(int x, int y);
  void close();
    
  void add_element(base_element&);
  void del_element(base_element&);
    
  list<elem_item>& all_elements() { return Elems; }
  
  bool is_member(base_element&);  
    
  
  void scene_begin(int = 0);
  void add_animation(anim_item);  
  int  scene_end();
    
  void scene_step(int = 1);
  void scene_finish();
  void scene_play();
  
  int  set_delay(int);
  int  get_delay() const;

  bool wait();
  bool wait(int);
  
  void add_action(action_item);
  void del_action(action_item);  
  
  void focus(base_element&) {}
  void unfocus() {}

  double xmin() const;
  double xmax() const;
  double ymin() const;
  double ymax() const;
};



//---------------------------------------------------------------------------
// class base_element
// ---------------------------------------------------------------------------

class base_element
{ protected:

  view*     V;    
  int       rank;
  list_item item;
  
  virtual void attach(view& v) { V = &v; };
  virtual void detach(view& v) { if (V == &v) V = 0; }  
  virtual bool is_attached() const { return V != 0; }
  virtual bool is_attached(view& v) const { return V == &v; }
 
  virtual void      set_item(list_item it);
  virtual list_item get_item() const;
 
  friend class view;
  
  public:
 
  base_element();
  base_element(const base_element& x);
  base_element& operator=(const base_element& x);
    
  virtual ~base_element() {}

  virtual void redraw() = 0;    
    
  virtual point     get_position() const = 0;
  virtual rectangle get_bounding_box() = 0;
  
  virtual int set_rank(int x);
  virtual int get_rank() const;  
  
  virtual view& get_view() const { return *V; }

  virtual bool intersect(segment,list<point>&) const { return false; }
    
  friend int compare(const base_element* x, const base_element* y)
  { if (x->get_rank() == y->get_rank()) return 0;
    if (x->get_rank() < y->get_rank()) return 1;
    return -1;
  }
};

typedef base_element* elem_item;

//---------------------------------------------------------------------------
// class relation
// ---------------------------------------------------------------------------

class base_relation
{

};

//---------------------------------------------------------------------------
// class action
// --------------------------------------------------------------------------

struct base_animation
{ 
  virtual void init(int) {}
  virtual int  steps() const = 0;
  
  virtual void start() {}
  virtual void step() {}
  virtual void finish() {}

  virtual ~base_animation() {}
};

typedef base_animation* anim_item;


//---------------------------------------------------------------------------
// class path
// ---------------------------------------------------------------------------

class path 
{
  motion_base* P;
  bool x;
  
  public:
  
  path();
  path(const path& p);

  template <class motion_t>
  path& operator=(const motion_t& M)
  { delete P; 
    P = new motion_t(M); 
    return *this; 
  }
  path& operator=(const path& p);
    
 ~path();
 
  template <class motion_t>
  path(const motion_t& M)  : P(new motion_t(M)), x(true) {}
  
  path(motion_base*) : P(new linear_motion), x(true) {}
  
  void   get_path(point p0, point p1, int steps, list<point>& L);
  double get_path_length(point p0, point p1);
  vector get_step(vector v, int steps, int i);
};

//---------------------------------------------------------------------------
// class base_move
// --------------------------------------------------------------------------

template <class element>
class base_move : public base_animation
{ 
  protected:
  
  element& S;       // element
  view&    V;       // view  
  point    p1;      // destination  
  path     Path;    // animation path   
  
  list<point> L;    // stores coordinates
  
  int      num;     // max. number of required steps
  
  public:

  base_move(element& s, point p) : S(s), V(S.get_view()), p1(p)
  { window& W = V.get_window();
    num = W.real_to_pix(Path.get_path_length(p1,S.get_position())); 
  }

  base_move(element& s, point p, const path& P) : S(s), V(S.get_view()), p1(p)
  { Path = P;  
    window& W = V.get_window();
    num = W.real_to_pix(Path.get_path_length(p1,S.get_position())); 
  }
  
  void init(int steps)
  { Path.get_path(S.get_position(),p1,steps,L); }
    
  int steps() const { return num; }  
  
  void step() { S.set_position(L.pop()); }
  
  void finish()
  { if (S.get_position() != p1) S.set_position(p1);
    delete this;
  }
};


//---------------------------------------------------------------------------
// class action
// ---------------------------------------------------------------------------

struct base_action
{ 
  virtual ~base_action() {}         

  enum 
  { ACT_LEFT      = (1 << 0),
    ACT_MIDDLE    = (1 << 1),
    ACT_RIGHT	    = (1 << 2),
    ACT_SHIFT	    = (1 << 3),
    ACT_CTRL	    = (1 << 4),
    ACT_ALT	      = (1 << 5),
    ACT_DOUBLE    = (1 << 6),
    ACT_DRAG      = (1 << 7),
   
    ACT_RET_TRUE  = (1 << 8),
    ACT_RET_FALSE = (1 << 9),
    ACT_CONTINUE  = (1 << 10)
  };
 
  virtual void    redraw() {}       
  virtual window* get_window() const = 0;
  virtual int     handle_event(int, int, double, double) = 0;
};

typedef base_action* action_item;

LEDA_END_NAMESPACE

#endif
