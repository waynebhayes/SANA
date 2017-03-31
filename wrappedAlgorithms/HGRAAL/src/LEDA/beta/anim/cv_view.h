/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  cv_view.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:33 $

#include <LEDA/core/list.h>
#include <LEDA/core/map.h>
#include <LEDA/graphics/window.h>
#include <LEDA/graphics/motion.h>
#include <LEDA/system/assert.h>


LEDA_BEGIN_NAMESPACE

class cv_base { // interface for all cv-viewers

public:

virtual bool wait() = 0;

virtual void new_element(unsigned id, double val, double x, double y) = 0;
virtual void new_pointer_element(unsigned s, unsigned t, double x, double y) = 0;
virtual void del_element(unsigned id) = 0;

virtual void draw() = 0;

virtual void scene_begin(int s=0) = 0;
virtual int  scene_end()          = 0;
virtual void scene_step()         = 0;
virtual void scene_finish()       = 0;


virtual void swap(unsigned i, unsigned j)   = 0;
virtual void assign(unsigned i, unsigned j) = 0;
virtual void assign(unsigned i, double val) = 0;
virtual void p_assign(unsigned, unsigned)   = 0;


virtual void set_cell_color(unsigned i, color col)       = 0;
virtual void set_value_color(unsigned i, color col)      = 0;
virtual void set_pointer_color(unsigned i, color col)    = 0;
virtual void translate(unsigned i, double dx, double dy) = 0;

virtual void focus(unsigned i) = 0;
virtual void unfocus() = 0;

virtual ~cv_base() {};
};



template<class element_t>
class dummy_pointer : public element_t {
public:
dummy_pointer()
{ element_t::set_cell_color(invisible);
  element_t::set_value_color(invisible);
}
dummy_pointer(const element_t&)
{ element_t::set_cell_color(invisible);
  element_t::set_value_color(invisible);
}
void assign(const element_t&) {}
void setnull() {}           
};




template<class view_t, class element_t,
                       class pointer_t=dummy_pointer<element_t> >
class cv_view : public cv_base, public view_t {

map<unsigned,element_t*> elem;
map<unsigned,pointer_t*> ptr;

motion_base*  default_motion;
motion_base*  swap_motion1;
motion_base*  swap_motion2;

public:

cv_view(int w, int h, string label) : view_t(w,h,label), 
                                      elem(0),
                                      ptr(0),
                                      default_motion(0),
                                      swap_motion1(0),
                                      swap_motion2(0) {}

bool wait() { return view_t::wait(); }

void new_pointer_element(unsigned id1, unsigned id2, double x, double y)
{ element_t* q = elem[id2];
  assert(q);
  pointer_t* p = new pointer_t(*q);
  p->set_position(x,y);
  p->set_value(0);
  view_t::add_element(*p);
  ptr[id1] = p;
}

void new_element(unsigned id, double val, double x, double y)
{ element_t* p = new element_t();
  p->set_position(x,y);
  p->set_value(val);
  view_t::add_element(*p);
  elem[id] = p;
}

void del_element(unsigned id) 
{ element_t* p = elem[id];
  elem[id] = 0;
  view_t::del_element(*p);
  delete p;
};

void draw()               { view_t::draw();             }
void scene_begin(int s=0) { view_t::scene_begin(s);     }
int  scene_end()          { return view_t::scene_end(); }
void scene_step()         { view_t::scene_step();       }
void scene_finish()       { view_t::scene_finish();     }


void swap(unsigned i, unsigned j)   
{ elem[i]->swap(*(elem[j]),swap_motion1,swap_motion2);   }

void assign(unsigned i, unsigned j) 
{ elem[i]->assign((*elem[j]),default_motion); }

void assign(unsigned i, double val) { elem[i]->assign(val);        }

void p_assign(unsigned i, unsigned j)
{ ptr[i]->assign(*(elem[j])); }

void p_setnull(unsigned i)
{ ptr[i]->setnull(); }

void set_cell_color(unsigned i, color col) { elem[i]->set_cell_color(col); }
void set_value_color(unsigned i, color col) { elem[i]->set_value_color(col); }
void set_pointer_color(unsigned i, color col) { ptr[i]->set_value_color(col); }

void translate(unsigned i, double dx, double dy) { elem[i]->translate(dx,dy); }

void focus(unsigned i) { view_t::focus(*elem[i]); }
void unfocus()         { view_t::unfocus(); }

void set_default_motion(motion_base* m) 
{ default_motion = m; }

void set_swap_motions(motion_base* m1, motion_base* m2) 
{ swap_motion1 = m1; swap_motion2 = m2; }

motion_base* get_default_motion() { return default_motion; }
motion_base* get_swap_motion1()   { return swap_motion1;    }
motion_base* get_swap_motion2()   { return swap_motion2;    }

};




class cv_dispatcher {

list<cv_base*> vw_list;

int   steps;
float speed;

public:

cv_dispatcher(float s = 1.0) : steps(0), speed(s) {} 

bool wait() 
{ if (vw_list.empty()) return true;
  else return vw_list.head()->wait(); 
}

void attach(cv_base& vw) { vw_list.append(&vw); }

void new_element(unsigned id, double val, double x, double y)
{ cv_base* cvp;
  forall(cvp,vw_list) cvp->new_element(id,val,x,y);
}

void new_pointer_element(unsigned s, unsigned t, double x, double y)
{ cv_base* cvp;
  forall(cvp,vw_list) cvp->new_pointer_element(s,t,x,y);
}

void draw()
{ cv_base* cvp;
  forall(cvp,vw_list) cvp->draw();
}

void scene_begin(int s=0)
{ cv_base* cvp;
  s = int(s/speed);
  forall(cvp,vw_list) cvp->scene_begin(s);
  steps = s;
}

void scene_end()
{ cv_base* cvp;
  forall(cvp,vw_list) cvp->scene_end();
}

void scene_play()
{ cv_base* cvp;
  while (steps--)
     forall(cvp,vw_list) cvp->scene_step();
  forall(cvp,vw_list) cvp->scene_finish();
}

void swap(unsigned i, unsigned j)
{ cv_base* cvp;
  forall(cvp,vw_list) cvp->swap(i,j);
}

void assign(unsigned i, unsigned j)
{ cv_base* cvp;
  forall(cvp,vw_list) cvp->assign(i,j);
}

void p_assign(unsigned i, unsigned j)
{ cv_base* cvp;
  forall(cvp,vw_list) cvp->p_assign(i,j);
}

void assign(unsigned i, double val)
{ cv_base* cvp;
  forall(cvp,vw_list) cvp->assign(i,val);
}

void set_cell_color(unsigned i, color col)
{ cv_base* cvp;
  forall(cvp,vw_list) cvp->set_cell_color(i,col);
}

void set_value_color(unsigned i, color col)
{ cv_base* cvp;
  forall(cvp,vw_list) cvp->set_value_color(i,col);
}

void set_pointer_color(unsigned i, color col)
{ cv_base* cvp;
  forall(cvp,vw_list) cvp->set_pointer_color(i,col);
}

void translate(unsigned i, double dx, double dy)
{ cv_base* cvp;
  forall(cvp,vw_list) cvp->translate(i,dx,dy);
}

void focus(unsigned i)
{ cv_base* cvp;
  forall(cvp,vw_list) cvp->focus(i);
}

void unfocus()
{ cv_base* cvp;
  forall(cvp,vw_list) cvp->unfocus();
}

void set_speed(float s) { speed = s; }

};


LEDA_END_NAMESPACE
