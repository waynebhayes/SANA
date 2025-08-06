#ifndef BOUNDS_H_
#define BOUNDS_H_
#include "../common.h"
// standard
#include <vector>
using std::vector;
// local
#include "../Hungarian/matrix.h"
#include "../qap.h"

double rearrangement_bound(vector<double> const& vec1,
		vector<double> const& vec2, bool is_lower);
double full_glb(QAP * problem, bool const is_lower);
double partial_glb(QAP * problem, bool const is_lower, vector<int>,
		vector<int>);
double two_level_glb(QAP * problem, bool const is_lower);
double tree_glb(QAP * problem, int const k, double const d,
		bool const is_lower);
void compute_rearrangement_bounds(Matrix<COST> const& cmat,
		QAP *const problem, bool const is_lower);
#endif /*BOUNDS_H_*/
