/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  plane_sweep.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:44:56 $

#ifndef LEDA_PLANE_SWEEP_H
#define LEDA_PLANE_SWEEP_H

#include<LEDA/core/list.h>
#include<LEDA/core/map.h>
#include<LEDA/system/event.h>

LEDA_BEGIN_NAMESPACE



template<class P, class Y, class X, class Out>
class plane_sweep 
{ 

public:

    typedef plane_sweep<P, Y,  X, Out> PSWEEP;
    typedef typename Y::item yitem;
    typedef typename X::item event;

    typedef typename Y::key_type ykey;
    typedef typename Y::inf_type yinf;
    typedef typename X::key_type xkey;
    typedef typename X::inf_type xinf;
    typedef Out      output;
    typedef P        input;
    typedef X        xstructure;
    typedef Y        ystructure;

  
    class sweep_cmp : public leda_cmp_base<ykey>
    {
      
      xkey p_sweep;
      int  (*sweep_compare_function)(const xkey&, const ykey&, const ykey&);
      
      public :

      sweep_cmp()
      {
         sweep_compare_function = compare_at_pos;
      }

      sweep_cmp(int (*f)(const xkey&, const ykey&, const ykey&)) 
      {
        sweep_compare_function = f;
      }

      virtual ~sweep_cmp(){}

      int operator()(const ykey& x, const ykey& y) const
      {   
        return sweep_compare_function(p_sweep, x, y);
      }
        
      void set_position(const xkey& p) { p_sweep = p; }
      xkey get_position() { return p_sweep; }
      
    };



  
    sweep_cmp    sw_pos;
    X            X_structure;
    Y            Y_structure;

    const P*     in;
    Out*         out;
    int          status;

    bool (*init_handler)(PSWEEP&, const P&);
    bool (*event_handler)(PSWEEP&);
    void (*animation_handler)(PSWEEP&);
    bool (*check_handler)(PSWEEP&);
    bool (*finish_handler)(PSWEEP&, const P&);

    map<yitem, event> SL_link;
    map<event, yitem> event_link;


  
  public :
  
  
  EVENT1<PSWEEP&>        start_sweep_event;
  EVENT1<PSWEEP&>        stop_sweep_event;
  EVENT1<PSWEEP&>        finish_sweep_event;

  EVENT2<PSWEEP&, event> new_event_event;
  EVENT2<PSWEEP&, event> del_event_event;

  EVENT2<PSWEEP&, yitem> new_yitem_event;
  EVENT2<PSWEEP&, yitem> del_yitem_event;


                     
  
  

    plane_sweep( const P* i = 0, Out* o = 0) : sw_pos(), X_structure(),
      Y_structure(sw_pos), in(i), out(o), status(0),
      init_handler(0), event_handler(0), animation_handler(0), check_handler(0), 
      finish_handler(0), SL_link(0), event_link(0) {}


    plane_sweep(int (*f)(const xkey&, const ykey&, const ykey&), 
          const P* i=0, Out* o=0) :
      sw_pos(f), X_structure(), Y_structure(sw_pos), in(i), out(o), status(0),
      init_handler(0), event_handler(0), animation_handler(0), check_handler(0), 
      finish_handler(0), SL_link(0), event_link(0) {}

   

  
  virtual ~plane_sweep() {}

  

    bool init( const P& L )
    { 
      if ( init_handler(*this, L) )
        {
          event ev = current_event();
          if (ev) sw_pos.set_position(X_structure.key(ev)); 
          else return true;

          if (animation_handler) animation_handler( (*this));
          if (check_handler) if (!check_handler(*this) ) return false;   
        }
      else return false;
      return true;
    }

    bool finish( const P& L ) { return finish_handler(*this, L); }
    bool sweep()
    {
      start_sweep_event.occur(*this);
      while ( !X_structure.empty() ) 
        {
          event ev = current_event();
          sw_pos.set_position(X_structure.key(ev));
          if ( !event_handler(*this) ) return false;
          next_event();
          if (animation_handler)  animation_handler( (*this));
          if (check_handler) if (!check_handler(*this) ) return false;
        }
      finish_sweep_event.occur(*this);
      return true;
    }

    bool animate(xkey stop, int del=1) 
    {
      start_sweep_event.occur(*this);
      if (animation_handler) 
        {
          animation_handler( (*this));
          leda_wait(del);
        }
      if (check_handler) if (!check_handler(*this) ) return false;   
          
      while ( !X_structure.empty() ) 
        {
          event ev = current_event();
          sw_pos.set_position(X_structure.key(ev));
          if ( compare(sw_pos.get_position(), stop) > 0 ) 
            {
              stop_sweep_event.occur(*this);
              return true;
            }
          if ( !event_handler(*this) ) return false;
          next_event();
          if (animation_handler)  
            {
               animation_handler( (*this));
               leda_wait(del);
            }
          if (check_handler) if (!check_handler(*this) ) return false;
        }
      finish_sweep_event.occur(*this);
      return true;
    }


  

    void set_init_handler(bool(*f)(PSWEEP&, const P&))  { init_handler = f; }
    void set_event_handler(bool(*f)(PSWEEP&))  { event_handler = f; }
    void set_finish_handler( bool(*f)(PSWEEP&, const P&)) { finish_handler = f; }
    void set_animation_handler(void(*f)(PSWEEP&))   { animation_handler = f; }
    void set_check_handler(bool(*f)(PSWEEP&))   { check_handler = f; }



  
    
    void  set_xref(yitem it, event ev) { SL_link[it] = ev; }
    event get_xref(yitem it)           { return SL_link[it]; }
    void  set_yref(event ev, yitem it) { event_link[ev] = it; }
    yitem get_yref(event ev)           { return event_link[ev]; }


  

    event new_event(xkey x, xinf xi) 
    { 
      event xit = X_structure.insert(x, xi);
      new_event_event.occur(*this, xit);
      return xit; 
    }
      
    void  del_event(event ev) 
    { 
      del_event_event.occur(*this, ev);

      yitem it = get_yref(ev);
      if (it) set_xref(it,0);

      X_structure.del_item(ev); 
    }


    event current_event()            { return X_structure.first_item(); }

    void  next_event() 
    { event ev = X_structure.first_item();
      if (ev) del_event(ev); 
     }

    xkey  event_key(event ev)        { return X_structure.key(ev); }

    xinf&  event_inf(event ev)        { return X_structure[ev]; }

    void  change_event(event ev, const xinf& xi) 
    { X_structure.change_inf(ev, xi);}
   
    xkey  sweep_position()           { return sw_pos.get_position(); }


  
   
    yitem  first_item() const            { return Y_structure.first_item(); }
    yitem  next_item(yitem it) const      { return Y_structure.next_item(it); }

    yitem  insert(ykey y , yinf yi)      
    { yitem yit = Y_structure.insert(y, yi);
      new_yitem_event(*this, yit);
      return yit; 
     }
      
    void  del_item(yitem it) 
    { del_yitem_event.occur(*this,  it);
      Y_structure.del_item(it); 
     }


    yitem  succ(yitem it) const           { return Y_structure.succ(it); }

    yitem  pred(yitem it) const           { return Y_structure.pred(it); }

    ykey  key (yitem it)                 { return Y_structure.key(it); }
    
    yinf&  inf(yitem it)                  { return Y_structure[it]; } 
    
    void  change_yitem(yitem it, const yinf& yi)  { Y_structure.change_inf(it, yi);; }

  

  output& get_output() { return *out; }
  void set_output(output* o) { out = o; }

  const input& get_input() { return *in; }
  void set_input(const input* p) { in = p; }

  int get_status() { return status; }
  void set_status(int s) { status = s; }



  xstructure&  get_xstructure() { return X_structure; }
  ystructure&  get_ystructure() { return Y_structure; }


};



LEDA_END_NAMESPACE

#endif

