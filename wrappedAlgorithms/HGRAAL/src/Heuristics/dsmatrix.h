#ifndef DSMATRIX_H_
#define DSMATRIX_H_
#include "../common.h"
// boost
#include <boost/dynamic_bitset.hpp>
using boost::dynamic_bitset;
#include <boost/numeric/ublas/matrix.hpp>
typedef boost::numeric::ublas::matrix<int> MATRIX;
// local
#include "../Random_Number_Generator/prng.h"
#include "../assignment.h"

class DSMATRIX {
	int _n;
	MATRIX _mat;
	int _row_col_sum;
	// matrix of bits indicating if a given _mat entry is a positive
	// integer
	vector<dynamic_bitset<> > _is_positive;

	bool check_mat();

public:
	DSMATRIX(int const n);
	~DSMATRIX();
	void fill(int const scalar);
	int get_sum() const;
	void add_assignment(ASSIGNMENT *const assigned, int const scalar);
	void add_assignments(vector<ASSIGNMENT *> assignments,
			vector<int> scalars);
	DSMATRIX & operator*=(int const scalar);
	DSMATRIX & operator+=(DSMATRIX const& rhs);
	ASSIGNMENT * remove_assignment(PRNG & prng);
	void remove_assignments(vector<ASSIGNMENT *> assignments,
			int const num_assignments, PRNG & prng);
};
#endif /*DSMATRIX_H_*/
