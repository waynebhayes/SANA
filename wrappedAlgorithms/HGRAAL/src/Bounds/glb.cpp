// local
#include "../Hungarian/munkres.h"
#include "glb.h"

void GLB::sort_matrix(Matrix<double> & m) {
    assert(m.columns() == _n);
    assert(m.rows() == _n);

    // replace diagonal entries with end-of-row
    // entries to make contiguous rows
    for (int row = 0; row < _n-1; ++row) { // last row already contiguous
        int const col = row;
        m.set(row, col, m.get(row, _n-1));
    }

    // sort rows
    vector<double> tmp_row(_n-1);
    for (int row = 0; row < _n; ++row) {
        for (int col = 0; col < _n-1; ++col)
            tmp_row[col] = m.get(row, col);
        sort(tmp_row.begin(), tmp_row.end());
        for (int col = 0; col < _n -1; ++col)
            m.set(row, col, tmp_row[col]);
    }
}

void GLB::compute_rearrangement_bounds(QAP * problem) {
    // sort rows for distance and flow matrices
    Matrix<double> sorted_dmat(_n, _n);
    for (int row = 0; row < _n; ++row)
        for (int col = 0; col < _n; ++col)
            sorted_dmat.set(row, col, problem->get_distance(row, col));
    sort_matrix(sorted_dmat);
    Matrix<double> sorted_fmat(_n, _n);
    for (int row = 0; row < _n; ++row)
        for (int col = 0; col < _n; ++col)
            sorted_fmat.set(row, col, problem->get_flow(row, col));
    sort_matrix(sorted_fmat);

    // compute bounds
    for (int loc = 0; loc < _n; ++loc)
        for (int fac = 0; fac < _n; ++fac) {
            double cost = _cmat.get(loc, fac);
            if (_is_lower) // lower bound
                for (int col = 0; col < (_n-1); ++col)
                    cost += sorted_dmat.get(loc, col) * sorted_fmat.get(fac, _n
                            -col -2);
            else
                // upper bound
                for (int col = 0; col < (_n-1); ++col)
                    cost += sorted_dmat.get(loc, col) * sorted_fmat.get(fac,
                            col);
            _cmat.set(loc, fac, cost);
        }
}

GLB::GLB(QAP * problem, Matrix<double> const& cmat, bool const is_lower) :
    _n(problem->get_size()), _cmat(cmat), _is_lower(is_lower) {

    // compute maximum cost so that _cmat can be inverted for computing
    // max_glb
    _max_cost = 0.0;
    for (int loc = 0; loc < _n; ++loc)
        for (int fac = 0; fac < _n; ++fac) {
            double cost = _cmat.get(loc, fac);
            if (cost > _max_cost)
                _max_cost = cost;
        }
    //print_costs();
}

GLB::~GLB() {
    _cmat.clear();
}

void GLB::print_costs() {
    cout << "Location (row), facility (col)." << endl;
    for (int loc = 0; loc < _n; ++loc) {
        for (int fac = 0; fac < _n; ++fac) {
            cout.setf(ios_base::fixed, ios_base::floatfield);
            cout.precision(3);
            cout.width(3);
            cout << _cmat.get(loc, fac) << ",";
        }
        cout << endl;
    }
    cout << endl;
}

void GLB::set_cost(int const row, int const col, double const cost) {
    assert(row >= 0);
    assert(row < _n);
    assert(col >= 0);
    assert(col < _n);
    assert(cost >= 0.0);
    _cmat.set(row, col, cost);
}

double GLB::get_cost(int const row, int const col) {
    assert(row >= 0);
    assert(row < _n);
    assert(col >= 0);
    assert(col < _n);
    return _cmat.get(row, col);
}

double GLB::solve(vector<int> & sol) {

    Matrix<double> d(_cmat);

    if (!_is_lower) // negate and add maximum cost
        for (int loc = 0; loc < _n; ++loc)
            for (int fac = 0; fac < _n; ++fac)
                _cmat.set(loc, fac, _max_cost - _cmat.get(loc, fac));

    Munkres m;
    m.solve(_cmat);

    double sum = 0.0;
    for (int loc = 0; loc < _n; ++loc)
        for (int fac = 0; fac < _n; ++fac)
            if (_cmat.get(loc, fac) == 0) {
                //cout.setf(ios_base::fixed, ios_base::floatfield);
                //cout << d(loc, fac) << endl;
                sol.push_back(fac);
                sum += d(loc, fac);
            }

    { // check assignment
        vector<int> tmp_sol = sol;
        sort(tmp_sol.begin(), tmp_sol.end());
        for (int fac = 0; fac < _n; ++fac)
            assert(tmp_sol[fac] == fac);
    }

    return sum;
}
