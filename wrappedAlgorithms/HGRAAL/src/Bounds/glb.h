#ifndef GLB_H_
#define GLB_H_
#include "../common.h"
// standard
#include <vector>
using std::vector;
#include "../Hungarian/matrix.h"
#include "../qap.h"

class GLB {
private:
	int const _n;
	Matrix<double> _cmat;
	bool const _is_lower;
	double _max_cost;

	void sort_matrix(Matrix<double> & m);
	void compute_rearrangement_bounds(QAP * problem);

public:
	GLB(QAP * problem, Matrix<double> const& cmat, bool const is_lower);
	~GLB();
	void print_costs();
	void set_cost(int const row, int const col, double const cost);
	double get_cost(int const row, int const col);
	double solve(vector<int> & sol);
};
#endif /*GLB_H_*/
