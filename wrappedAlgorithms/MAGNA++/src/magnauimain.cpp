#include "magnaui.h"

//    #define PRINTFERR
//    void printferr(const char *fmt, ...) {
//    char s[1024];
//    va_list argptr;
//    va_start(argptr, fmt);
//    vsnprintf(s, 1024, fmt, argptr);
//    fl_message("In function %s. Tentative Error: %s. MAGNA will exit now.", s, strerror(errno));
//    va_end(argptr);
//}

void mg_error(const char *fmt, ...) {
    va_list arg;
    va_start(arg,fmt);
    vsnprintf(mg_errstr,1024,fmt,arg);
    va_end(arg);
    magnadonep = 1;
    magnaerrorp = 1;
}

// Basically every time this is called in the run_simulation function (or anywhere), it just jumps back the the main GUI loop
// apparently calling this from inside a thread is bad :(
jmp_buf exitjmpbuf;    
char mg_errstr[1024];
void mg_quit(int status) {
    longjmp(exitjmpbuf,1);
}

int numprocs() {
#ifdef _WIN32
#ifndef _SC_NPROCESSORS_ONLN
    SYSTEM_INFO info;
    GetSystemInfo(&info);
#define sysconf(a) info.dwNumberOfProcessors
#define _SC_NPROCESSORS_ONLN
#endif
#endif
    return sysconf(_SC_NPROCESSORS_ONLN);
}

int magnadonep = 0;
int magnaerrorp = 0;
    

void *fn_run_simulation(void *v) {
    //    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
    //    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
    MagnaUI* app = (MagnaUI*)v;
    struct magnaparams *sim = (struct magnaparams*)app->sim;
    Fl_Double_Window *main_window = (Fl_Double_Window *)app->main_window;
    //    Fl_Button *stop_sim_btn = (Fl_Button *)(((MagnaUI*)v)->stop_sim_btn);
    //    if (!setjmp(exitjmpbuf)) {
        main_window->label("MAGNA++ is running");
        main_window->deactivate();
        //stop_sim_btn->activate();

        run_simulation(sim->graph_file_name1, sim->graph_file_name2, sim->init_pop_file,
                       sim->output_file_prefix, sim->edge_optimizing_measure_code,
                       sim->pop_size, sim->pop_elite, sim->n_gen, sim->freq,
                       sim->alpha, sim->nodematrix_file, sim->n_threads);
        magnadonep = 1;
        //    }
    //error when i run the below. guess fltk is thread unsafe?
    //fl_message("MAGNA++ has finished running.");
    //setjmp(exitjmpbuf);
}


//pthread_t run_sim_thread;
// pthread_t magnathread;
// pthread_t magnawaitthread;
//pthread_mutex_t magnawaitlock = PTHREAD_MUTEX_INITIALIZER;
//pthread_cond_t magnawaitcond;
// 
// void* s_run_simulation_wait(void* v) {
//     Fl_Double_Window *main_window = (Fl_Double_Window *)v;
//     pthread_join(magnathread,NULL);
//     main_window->label("MAGNA++");
//     fl_message("MAGNA++ has finished running.");    
//     //    pthread_mutex_lock(&magnawaitlock);
//     //    pthread_cond_wait(&magnawaitcond, &magnawaitlock);
//     //    pthread_cond_unlock(&magnawaitlock);
// }
// //void* s_run_simulation_done(void*) {
// //    pthread_mutex_lock(&magnawaitlock);
// //    pthread_cond_signal(&magnawaitcond);
// //    pthread_cond_unlock(&magnawaitlock);
// //}

int main(int argc, char** argv) {
    MagnaUI *app = new MagnaUI;
    app->show(argc,argv);
    //    return Fl::run();
    while(1) {
        setjmp(exitjmpbuf);
        if (!Fl::wait()) break;
        if (magnadonep) {
            app->main_window->label("MAGNA++");
            app->main_window->activate();
            if (!magnaerrorp) {
                //fl_message("MAGNA++ has finished running.");
                fl_message(mg_finalstr);
            }            
            else
                fl_message(mg_errstr);                
                magnaerrorp = 0;
            magnadonep = 0;
        }
    }
}
