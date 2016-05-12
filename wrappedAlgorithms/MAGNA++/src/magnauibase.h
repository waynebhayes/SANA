#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <setjmp.h>
#include <pthread.h>
#include <sys/stat.h>

#undef Success
extern jmp_buf exitjmpbuf;    
extern char mg_errstr[1024];
#include "genetic.h"


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

//extern pthread_t magnathread;
//extern pthread_t magnawaitthread;
//extern Fl_Thread run_sim_thread;
//extern pthread_t run_sim_thread;
void *fn_run_simulation(void *v);
//void *s_run_simulation_wait(void *v);
//extern pthread_mutex_t magnawaitlock;
//extern pthread_cond_t magnawaitcond;
extern int magnadonep;
extern int magnaerrorp;

int numprocs();

class Fl_Lighter_Button : public Fl_Light_Button {
 public:
 Fl_Lighter_Button(int x, int y, int w, int h, const char *t) : Fl_Light_Button(x,y,w,h,t) { }
    int handle(int event) {
        if (event==FL_RELEASE) {
            do_callback();
            return 1;
        }
        else if (event==FL_PUSH) {
            Fl_Light_Button::handle(FL_PUSH);
            setonly();
        }
        else
            return Fl_Light_Button::handle(event);
    }
};
