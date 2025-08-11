#include "../common.h"
// standard
#include <fstream>
using std::ifstream;
#include <algorithm>
#include <vector>
using std::vector;
// local
#include "histogram.h"
#include "../Hungarian/matrix.h"
#include "../Bounds/bounds.h"
#include "../Random_Number_Generator/prng.h"

void statistics() {
    int const ntrials = 1000000000;

    string msg("::run_bounds");

    QAP * problem;
    try {
        problem = new QAP("Problems/bur26b.dat");
    } catch(...) {
        cout << (msg + "error reading problem.");
        throw;
    }
    int const n = problem->get_size();
    int const npairs = n*(n-1);

    // bool normalize = false;

    // compute max, min and range for distance and flow matrices
    double dmin = problem->get_min_distance();
    double dmax = problem->get_max_distance();
    double fmin = problem->get_min_flow();
    double fmax = problem->get_max_flow();
    double const drange = dmax-dmin;
    double const frange = fmax-fmin;

    /*
     if (normalize) {
     // normalize matrix entries to lie within unit interval
     if ((drange == 0.0) || (drange == 0.0)) // replace with epsilon later
     return; // degenerate problem where all entries are the same
     else
     for (int row = 0; row < n; ++row)
     for (int col = 0; col < n; ++col)
     if (row != col) {
     dmat(row, col) = (problem->get_distance(row, col)-dmin)/drange;
     fmat(row, col) = (problem->get_flow(row, col)-fmin)/frange;
     }
     }
     */

    // compute mean for distance and flow matrices
    double dmean = problem->get_mean_distance();
    double fmean = problem->get_mean_flow();
    double mean_cost = (n-1)*dmean*fmean;

    /*
     if (normalize)
     // normalized with respect to npairs
     mean_cost /= (n-1);
     */

    // compute node distance and flow averages
    vector<double> node_davg(n), node_favg(n);
    for (int row = 0; row < n; ++row) {
        for (int col = 0; col < n; ++col)
            if (row != col) {
                node_davg[row] += problem->get_distance(row, col);
                node_favg[row] += problem->get_flow(row, col);
            }
        node_davg[row] /= (n-1);
        node_favg[row] /= (n-1);
    }

    // compute Gilmore-Lawler bounds for QAP cost
    // normalized with respect to npairs
    double min_glb = full_glb(problem, true);
    double max_glb = full_glb(problem, false);

    /*
     if (normalize) {
     min_glb /= npairs;
     max_glb /= npairs;
     }
     */

    cout.setf(ios_base::fixed, ios_base::floatfield);
    cout << "GLB lower bound: " << min_glb << endl;
    cout << "GLB upper bound: " << max_glb << endl;

    // compute rearrangement bounds for approximate QAP cost
    // normalized with respect to npairs
    double min_rgb = (n-1)*rearrangement_bound(node_davg, node_favg,
            true);
    double max_rgb = (n-1)*rearrangement_bound(node_davg, node_favg,
            false);

    /*
     if (normalize) {
     min_rgb /= npairs;
     max_rgb /= npairs;
     }
     */

    cout << "Rearrangement lower bound: " << min_rgb << endl;
    cout << "Rearrangement upper bound: " << max_rgb << endl;

    /* [1] Compute statistical distribution of QAP costs */
    if (true) {
        // initialize histogram
        Histogram hist(min_glb, max_glb, 40, 40);
        int const perms_per_hist = 1000000;
        int cnt = 0;

        // generate seed array for input to random number generator
        PRNG prng("QAP Simulation Distribution Experiment Run 1");

        // initialize permutation to the identity permutation
        vector<int> perm(n);
        for (int i = 0; i < n; ++i)
            perm[i] = i;

        // evaluate QAP cost function on random permutations of the flow
        // matrix
        vector<double> dotprod;
        for (int k = 0; k < ntrials; ++k) {
            // randomize permutation
            random_shuffle(perm.begin(), perm.end(), prng);

            // evaluate QAP cost function
            double qap_cost = 0.0;
            for (int row = 0; row < n; ++row)
                for (int col = 0; col < n; ++col)
                    if (row != col)
                        qap_cost += problem->get_distance(row, col)
                                * problem->get_flow(perm[row], perm[col]);

            /*
             if (normalize)
             qap_cost /= npairs; // normalized with respect to npairs
             */

            dotprod.push_back(qap_cost); // store cost

            // histogram accumulation and display step (once every large
            // number of permutations)
            if (++cnt == perms_per_hist) {
                cout << endl << "Mean cost: " << mean_cost << endl;
                hist.accumulate(dotprod);
                hist.display();
                dotprod.clear();
                cnt = 0;
            }

            // reset to identity permutation
            for (int i = 0; i < n; ++i)
                perm[i] = i;
        }

        // final histogram accumulation and display
        cout << endl << "Mean cost: " << mean_cost << endl;
        hist.accumulate(dotprod);
        hist.display();
    }

    /* [2] Compute statistical distribution of approximate QAP costs */
    if (false) {
        // initialize histogram
        Histogram hist(min_rgb, max_rgb, 40, 40);
        int const perms_per_hist = 1000000;
        int cnt = 0;

        // generate seed array for input to random number generator
        PRNG prng("QAP Simulation Distribution Experiment Run 1");

        // initialize permutation to the identity permutation
        vector<int> perm(n);
        for (int i = 0; i < n; ++i)
            perm[i] = i;

        // evaluate QAP cost function on random permutations of the flow
        // matrix
        vector<double> dotprod;
        for (int k = 0; k < ntrials; ++k) {
            // randomize permutation
            random_shuffle(perm.begin(), perm.end(), prng);

            // evaluate QAP cost function approximated by node distance
            // and flow averages
            double approx_cost = 0.0;
            for (int row = 0; row < n; ++row)
                approx_cost += node_davg[row] * node_favg[perm[row]];
            /*
             if (normalize)
             approx_cost /= n; // normalized with respect to npairs
             */

            dotprod.push_back(approx_cost); // store cost

            // histogram accumulation and display step (once every large
            // number of permutations)
            if (++cnt == perms_per_hist) {
                cout << endl << "Mean cost: " << mean_cost << endl;
                hist.accumulate(dotprod);
                hist.display();
                dotprod.clear();
                cnt = 0;
            }

            // reset to identity permutation
            for (int i = 0; i < n; ++i)
                perm[i] = i;
        }

        // final histogram accumulation and display
        cout << endl << "Mean cost: " << mean_cost << endl;
        hist.accumulate(dotprod);
        hist.display();
    }

    /* [3] Compute statistical distribution of differences between QAP costs */
    if (false) {
        // compute difference between Gilmore-Lawler and rearrangement
        // bounds for QAP cost normalized with respect to npairs
        double min_cost = min_glb - max_rgb;
        cout << "Difference lower bound: " << min_cost << endl;
        double max_cost = max_glb - min_rgb;
        cout << "Difference upper bound: " << max_cost << endl;;

        // initialize histogram
        Histogram hist(min_cost, max_cost, 40, 40);
        int const perms_per_hist = 1000000;
        int cnt = 0;

        // generate seed array for input to random number generator
        PRNG prng("QAP Simulation Distribution Experiment Run 1");

        // initialize permutation to the identity permutation
        vector<int> perm(n);
        for (int i = 0; i < n; ++i)
            perm[i] = i;

        // evaluate QAP cost function on random permutations of the flow
        // matrix
        vector<double> dotprod;
        for (int k = 0; k < ntrials; ++k) {
            // randomize permutation
            random_shuffle(perm.begin(), perm.end(), prng);

            // evaluate QAP cost function
            double qap_cost = 0.0;
            for (int row = 0; row < n; ++row)
                for (int col = 0; col < n; ++col)
                    if (row != col)
                        qap_cost += problem->get_distance(row, col)
                                * problem->get_flow(perm[row], perm[col]);
            qap_cost /= npairs; // normalized with respect to npairs

            // evaluate QAP cost function approximated by node distance
            // and flow averages
            double approx_cost = 0.0;
            for (int row = 0; row < n; ++row)
                approx_cost += node_davg[row] * node_favg[perm[row]];
            /*
             if (normalize)
             approx_cost /= n; // normalized with respect to npairs
             */

            dotprod.push_back(qap_cost - approx_cost); // store cost
            cout << qap_cost << " " << approx_cost << endl;

            // histogram accumulation and display step (once every large
            // number of permutations)
            if (++cnt == perms_per_hist) {
                cout << endl << "Mean cost: " << mean_cost << endl;
                hist.accumulate(dotprod);
                hist.display();
                dotprod.clear();
                cnt = 0;
            }

            // reset to identity permutation
            for (int i = 0; i < n; ++i)
                perm[i] = i;
        }

        // final histogram accumulation and display
        cout << endl << "Mean cost: " << mean_cost << endl;
        hist.accumulate(dotprod);
        hist.display();
    }
}
