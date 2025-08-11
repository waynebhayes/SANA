/* in here goes all of the code that i want to put in magnaui.fl
   but is too big to properly edit in magnaui.fl */

#ifdef run_sim_btn_cb_i_fn
// void MagnaUI::run_sim_btn_cb_i(Fl_Button* w,void* v) {
// This is where we check everything and run MAGNA

if (!file_exists(sim->graph_file_name1)) {
    mg_error("\"Network 1\". No such file: %s", sim->graph_file_name1);
    return;
 }
if (!file_exists(sim->graph_file_name2)) {
    mg_error("\"Network 2\". No such file: %s", sim->graph_file_name2);
    return;
 }

int len = strlen(sim->output_file_dir)-1;
if (sim->output_file_dir[len]=='/') sim->output_file_dir[len]='\0';
if (sim->output_file_dir[len]=='\\') sim->output_file_dir[len]='\0';
int dirstat = is_dir(sim->output_file_dir);
if (dirstat==-1) {
    mg_error("\"Output File Directory\". No such directory: %s", sim->output_file_dir);
    return;
 }
if (dirstat==0) {
    mg_error("\"Output File Directory\". Not a directory: %s", sim->output_file_dir);
    return;
 }

if (sim->output_file_prefix) free(sim->output_file_prefix);
sim->output_file_prefix =
    (char*)malloc(sizeof(char)*(5+strlen(sim->output_file_dir)+
                                strlen(sim->output_file_pref)));
sprintf(sim->output_file_prefix, "%s/%s",sim->output_file_dir,sim->output_file_pref);

if (sim->init_pop_file != NULL) {
    if (!file_exists(sim->init_pop_file)) {
        mg_error("\"Initial Population File\". No such file: %s",sim->init_pop_file);
        return;
    }
 }

if (sim->alpha < 0 || sim->alpha > 1) {
    mg_error("\"Edge-Node Weight\" needs to be between 0 and 1 inclusive");
    return;
 }
if (sim->use_nodematrix) {
    if (!file_exists(sim->nodematrix_file)) {
        mg_error("\"Node Comparison File\". No such file: %s", sim->nodematrix_file);
        return;
    }
 }
if (sim->pop_size < 0) {
    mg_error("\"Population Size\" must be a positive integer");
    return;
 }
if (sim->n_gen < 0) {
    mg_error("\"Number of Generations\" must be a positive integer");
    return;
 }
if (sim->pop_elite_ratio >= 0 && sim->pop_elite_ratio <= 1 ) {
    sim->pop_elite = sim->pop_elite_ratio * sim->pop_size;
 }
 else {
     mg_error("\"Fraction of Elite Members\" must be between 0 and 1 exclusive");
     return;
 }
if (sim->freq < 0 || sim->freq > sim->n_gen) {
    mg_error("\"Frequency of Output\" must be a non-negative integer no more than the number of generations");
    return;
 }

if ((sim->freq!=0) && (sim->n_gen % sim->freq != 0)) {
    fl_message("You probably want the \"Frequency of Output\" to divide the \"Number of Generations\".");
 }
if (sim->n_threads < 1) {
    mg_error("\"Number of Threads\" must be a positive integer");
    return;
 }
if (sim->n_threads > sim->pop_size) {
    sim->n_threads = sim->pop_size;
 }

// if (write_simulation_parameters(sim) == 0) {
//     return;
//  }

#if 1
//defined(_WIN32) && !defined(__CYGWIN__)

//if(!setjmp(exitjmpbuf)) {
    main_window->label("MAGNA++ is running");
    run_simulation(sim->graph_file_name1, sim->graph_file_name2, sim->init_pop_file,
                   sim->output_file_prefix, sim->edge_optimizing_measure_code,
                   sim->pop_size, sim->pop_elite, sim->n_gen, sim->freq,
                   sim->alpha, sim->nodematrix_file, sim->n_threads);
    magnadonep = 1;
// }

#else
// this is an attempt at having the gui be responsive
// while run_simulation is running
// but it's unstable (probly cus fltk can't handle threads?)
int ret = pthread_create(&run_sim_thread,NULL,fn_run_simulation,(void*)this);
if (ret) {
    mg_error("Error creating main thread. Too many threads?");
    return;
 }
pthread_detach(run_sim_thread);

#endif

#endif //run_sim_btn_cb_i_fn
