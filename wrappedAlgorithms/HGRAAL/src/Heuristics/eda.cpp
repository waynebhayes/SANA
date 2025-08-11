/*
 * Program: Implementation of proposed heuristic for the
 *          Quadratic Assignment Problem (QAP) for the
 *          purpose of minimizing overall communication cost
 *          of distributed computing
 * Course: ICS242 Distributed Computing Systems
 * Organization: UC Irvine
 * Author: Ng Weng Leong
 * Date: 11 Feb 2006
 */
#include "../common.h"
// standard
#include <fstream>
using std::ifstream;
#include <cstdlib>
#include <cmath>
// local
#include "../Prob/prob.h"

typedef unsigned int UINT;

// assume WLOG that all latencies and data rates are
// non-negative integers

// structure for a possible solution
typedef struct {
    int * nodes_assigned;
    int cost;
} SOL;

// comparison function for sorting
int compare(void const* pa, void const* pb) {
    SOL const* a = (SOL const*)(pa);
    SOL const* b = (SOL const*)(pb);
    return ((a->cost) - (b->cost));
}

// local search routine
void local_search(SOL *const sol, int **const dmat, int **const lmat,
        int const prob_sz, int *const seed, int const ntries) {
    SOL best_sol;
    best_sol.nodes_assigned = new int[prob_sz];
    for (int task = 0; task < prob_sz; ++task)
        best_sol.nodes_assigned[task] = sol->nodes_assigned[task];
    best_sol.cost = sol->cost;
    assert(sol->cost >= 0);

    for (int tries = 0; tries < ntries; ++tries) {
        // randomly pick a pair of nodes
        int const m = uniform_discrete_sample(0, prob_sz-1, seed);
        ++(*seed);
        int const n = (uniform_discrete_sample(0, prob_sz-2, seed) + m)
                %prob_sz;
        ++(*seed);

        // swap nodes
        int node = sol->nodes_assigned[m];
        sol->nodes_assigned[m] = sol->nodes_assigned[n];
        sol->nodes_assigned[n] = node;

        // compute the cost of the solution
        int cost = 0;
        for (int i = 0; i < prob_sz; ++i)
            for (int j = 0; j < prob_sz; ++j)
                cost += dmat[i][j] * lmat
                [sol->nodes_assigned[i]]
                [sol->nodes_assigned[j]];

        assert(cost >= 0);

        if (best_sol.cost > cost) {
            for (int task = 0; task < prob_sz; ++task)
                best_sol.nodes_assigned[task] = sol->nodes_assigned[task];
            best_sol.cost = cost;
        } else {
            // unswap nodes
            int node = sol->nodes_assigned[n];
            sol->nodes_assigned[n] = sol->nodes_assigned[m];
            sol->nodes_assigned[m] = node;
        }
    }

    for (int task = 0; task < prob_sz; ++task)
        sol->nodes_assigned[task] = best_sol.nodes_assigned[task];
    sol->cost = best_sol.cost;
    assert(sol->cost >= 0);

    delete[] best_sol.nodes_assigned;
}

void eda() {
    int const num_iter = 5000;
    int const samp_sz = 1000;
    int const skim_num = 100;
    double const new_prop = 0.2;
    ifstream ifs("nug30.dat");

    // read in distance and flow matrices
    int prob_sz;
    ifs >> prob_sz;
    assert(prob_sz > 0);
    int **const dmat = new int*[prob_sz];
    for (int i = 0; i < prob_sz; ++i) {
        dmat[i] = new int[prob_sz];
        for (int j = 0; j < prob_sz; ++j)
            ifs >> dmat[i][j];
    }
    int **const lmat = new int*[prob_sz];
    for (int i = 0; i < prob_sz; ++i) {
        lmat[i] = new int[prob_sz];
        for (int j = 0; j < prob_sz; ++j)
            ifs >> lmat[i][j];
    }

    double const old_prop = 1-new_prop;
    double const sum_epsilon = 1.0/(prob_sz*prob_sz*prob_sz);
    double const epsilon = sum_epsilon/prob_sz;

    int seed = 1;

    // initialize task-to-node distribution
    // to be uniform distributions for each task
    double * tdist = new double[prob_sz];
    double ** tndist = new double *[prob_sz];
    double ** work_tndist = new double *[prob_sz];
    for (int task = 0; task < prob_sz; ++task) {
        tndist[task] = new double[prob_sz];
        work_tndist[task] = new double[prob_sz];
        for (int node = 0; node < prob_sz; ++node)
            tndist[task][node] = 1.0/prob_sz;
    }

    // initialize task-to-node assignment
    // matrix and cost array
    SOL * solmat = new SOL[skim_num+samp_sz];
    for (int sample = 0; sample < (skim_num+samp_sz); ++sample)
        solmat[sample].nodes_assigned = new int[prob_sz];
    double node_labels[prob_sz];
    for (int n = 0; n < prob_sz; ++n)
        node_labels[n] = static_cast<double>(n);

    double ent[num_iter];
    for (int iter = 0; iter < num_iter; ++iter) {
        // (1) Solution Generation Step:
        // generate a bunch of solutions assigning each task to
        // each node based on the current distribution <tndist>

        UINT k = (iter > 0) ? skim_num : 0;

        for (int sample = 0; sample < samp_sz; ++sample) {
            // (1.1) generate one solution and record it

            // make a working copy of <tndist>
            for (int task = 0; task < prob_sz; ++task)
                for (int node = 0; node < prob_sz; ++node)
                    work_tndist[task][node] = tndist[task][node];

            for (int task = 0; task < prob_sz; ++task) {
                // randomly select a node based on the
                // given task-to-node distribution

                // <discrete_sample> index starts from 1
                int selected_node = discrete_sample(prob_sz,
                        work_tndist[task], &seed)-1;
                ++seed;
                solmat[k].nodes_assigned[task] = selected_node;

                // condition <work_tndist> based
                // on newly selected node
                for (int t = (task+1); t < prob_sz; ++t)
                    work_tndist[t][selected_node] = 0.0;
            } // task

            // perform a local search to improve solution first (new feature)
            local_search(solmat+k, dmat, lmat, prob_sz, &seed, 100);

            for (int i = 0; i < (prob_sz-1); ++i)
                for (int j = i+1; j < prob_sz; ++j)
                    assert(solmat[k].nodes_assigned[i] !=
                            solmat[k].nodes_assigned[j]);

            // (1.2) compute the cost of the above solution
            solmat[k].cost = 0;
            for (int i = 0; i < prob_sz; ++i)
                for (int j = 0; j < prob_sz; ++j)
                    solmat[k].cost += dmat[i][j]
                            * lmat[solmat[k].nodes_assigned[i]]
                            [solmat[k].nodes_assigned[j]];

            // (1.3) only keep solutions that beat some current-
            // best solution. Helps to keep sorting costs down.
            if (((iter > 0) && (solmat[k].cost < solmat[skim_num-1].cost))
                    || (iter == 0))
                ++k;
        } // sample

        // (2) Distribution Updating Step:
        // use the best solutions generated in
        // this iteration to update <tndist>

        // (2.1) sort all solutions
        qsort(solmat, k, sizeof(SOL), compare);

        // (2.2) remove duplicates
        int v = 1;
        for (int s = 0; v < skim_num; ++s) {
            int c = solmat[s].cost;
            bool stat = false;
            for (int w = v-1; w >= 0; --w) {
                if (solmat[w].cost != c)
                    break;
                int n;
                for (n = 0; n < prob_sz; ++n)
                    if (solmat[w].nodes_assigned[n]
                            != solmat[s].nodes_assigned[n])
                        break;
                if (n == prob_sz) {
                    stat = true;
                    break;
                }
            }
            if (stat == false) {
                SOL tmp = solmat[v];
                solmat[v] = solmat[s];
                solmat[s] = tmp;
                ++v;
            }
        }

        // (2.3) re-calculate <tndist>

        // compute frequency distribution of
        // nodes assigned for each task
        for (int t = 0; t < prob_sz; ++t) {
            for (int n = 0; n < prob_sz; ++n)
                tdist[n] = 0.0;
            for (int s = 0; s < skim_num; ++s)
                tdist[solmat[s].nodes_assigned[t]] += 1.0;
            for (int n = 0; n < prob_sz; ++n) {
                // mix old and new distributions via a weighted sum
                tndist[t][n] = old_prop * tndist[t][n] + new_prop
                        * empirical_discrete_pdf(n, prob_sz, tdist, node_labels);
                tndist[t][n] += epsilon;
                tndist[t][n] /= 1+sum_epsilon;
            }
        }

        // compute convergence measure
        // (average normalized entropy) on <tndist>
        ent[iter] = 0.0;
        for (int t = 0; t < prob_sz; ++t)
            for (int n = 0; n < prob_sz; ++n)
                ent[iter] -= tndist[t][n] * log(tndist[t][n]);
        ent[iter] /= prob_sz * log(prob_sz);
        cout << "[" << iter << "] ANE: " << ent[iter] << " Cost: "
                << solmat[0].cost << endl;
    } // iter

    // local search
    cout << "Performing local search ... " << endl;
    SOL best_sol;
    best_sol.nodes_assigned = new int[prob_sz];

    local_search(solmat, dmat, lmat, prob_sz, &seed, 1000);
    for (int task = 0; task < prob_sz; ++task)
        best_sol.nodes_assigned[task] = solmat[0].nodes_assigned[task];
    best_sol.cost = solmat[0].cost;
    for (int k = 1; k < skim_num; ++k) {
        local_search(solmat+k, dmat, lmat, prob_sz, &seed, 1000);
        if (best_sol.cost > solmat[k].cost) {
            for (int task = 0; task < prob_sz; ++task)
                best_sol.nodes_assigned[task]
                        = solmat[k].nodes_assigned[task];
            best_sol.cost = solmat[k].cost;
        }
    }

    cout << endl;
    cout << "Permutation: ";
    for (int task = 0; task < prob_sz; ++task)
        cout << best_sol.nodes_assigned[task] << " ";
    cout << endl;
    cout << "Cost: " << best_sol.cost << endl;
}
