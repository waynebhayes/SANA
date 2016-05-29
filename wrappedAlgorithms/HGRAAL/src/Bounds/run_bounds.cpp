#include "../common.h"
// standard
#include <fstream>
using std::ifstream;
#include <algorithm>
#include <vector>
using std::vector;
// local
#include "../Experiments/histogram.h"
#include "../Hungarian/matrix.h"
#include "../Bounds/bounds.h"
#include "../Random_Number_Generator/prng.h"
#include "../qap.h"

// compute Gilmore-Lawler bounds for QAP cost
// normalized with respect to npairs
void run_full_glb(QAP * problem) {
	double min_glb = full_glb(problem, true);
	double max_glb = full_glb(problem, false);

	cout.setf(ios_base::fixed, ios_base::floatfield);
	cout << "GLB lower bound: " << min_glb << endl;
	cout << "GLB upper bound: " << max_glb << endl;
}

void run_rgb(QAP * problem) {
	// compute node distance and flow averages
	int const n = problem->get_size();
	vector<double> node_dmean(n), node_fmean(n);
	for (int row = 0; row < n; ++row) {
		for (int col = 0; col < n; ++col)
			if (row != col) {
				node_dmean[row] += problem->get_distance(row, col);
				node_fmean[row] += problem->get_flow(row, col);
			}
		node_dmean[row] /= (n-1);
		node_fmean[row] /= (n-1);
	}

	// compute rearrangement bounds for approximate QAP cost
	// normalized with respect to npairs
	double min_rgb = (n-1)*rearrangement_bound(node_dmean, node_fmean,
			true);
	double max_rgb = (n-1)*rearrangement_bound(node_dmean, node_fmean,
			false);

	cout.setf(ios_base::fixed, ios_base::floatfield);
	cout << "Rearrangement lower bound: " << min_rgb << endl;
	cout << "Rearrangement upper bound: " << max_rgb << endl;
}

void run_tree_glb(QAP * problem) {
	double const min_glb = tree_glb(problem, 6, 1.0/29, true);
	//double const max_glb = tree_glb(problem, 1, 0.5, false);

	cout.setf(ios_base::fixed, ios_base::floatfield);
	cout << "Tree GLB lower bound: " << min_glb << endl;
	//cout << "Tree GLB upper bound: " << max_glb << endl;
}

void run_bounds() {
	string msg("::run_bounds");

	QAP * problem;
	try {
		problem = new QAP("Problems/nug30.dat");
	} catch(...) {
		cout << (msg + "error reading problem.");
		throw;
	}

	//run_full_glb(problem);
	//run_rgb(problem);
	run_tree_glb(problem);
}
