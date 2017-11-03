#include "common.h"

void statistics();
void optimalBits();
void lap();
void eda();
void testSHA2();
void test_glb();
void test_boost();
void run_new_eda();
void run_bounds();
void run_lap_approx();
void run_network_align();
void run_simple_network_align();
void run_advanced_network_align();
void run_compute_local_subgraph();
void find_one_optimal_alignment(string const res_align_file,
        string const res_paired_cost_matrix_file,
        string const paired_cost_matrix_file);

#if 0
bool check_asserts_on(){
    cout << "!!!Assertions are on!!!" << endl;
    return true;
}
#endif

#if 0
int main() {
    assert(check_asserts_on());
    //run_new_eda();
    //statistics();
    //testSHA2();
    //optimalBits();
    //lap();
    //eda();
    //test_glb();
    //test_boost();
    //run_bounds();
    //run_lap_approx();
    run_network_align();
    //run_simple_network_align();
    //run_advanced_network_align();
    //run_compute_local_subgraph();
}
#endif

int main(int argc, char * argv[]) {
    string res_align_file = string(argv[1]);
    string res_paired_cost_matrix_file = string(argv[2]);
    string paired_cost_matrix_file = string(argv[3]);
    cout << "Experiment: " << res_align_file << endl;
    find_one_optimal_alignment(res_align_file,
            res_paired_cost_matrix_file, paired_cost_matrix_file);
    cout << endl;
}
