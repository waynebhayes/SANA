// boost
#include <boost/numeric/ublas/matrix.hpp>
typedef boost::numeric::ublas::zero_matrix<int> ZERO_MATRIX;
#include <boost/numeric/ublas/vector.hpp>
typedef boost::numeric::ublas::vector<int> VEC;
typedef boost::numeric::ublas::scalar_vector<int> SCALAR_VEC;
// local
#include "dsmatrix.h"

// private member functions
bool DSMATRIX::check_mat() {
    SCALAR_VEC const ones(_n, 1);
    VEC const row_sums(prod(_mat, ones));
    VEC const col_sums(prod(trans(_mat), ones));
    for (int i = 0; i < _n; ++i)
        if ((row_sums[i] != _row_col_sum) ||(col_sums[i] != _row_col_sum))
            return false;
    return true;
}

// public member functions
DSMATRIX::DSMATRIX(int const n) :
    _n(n), _mat(ZERO_MATRIX(n, n)), _row_col_sum(0) {
#ifdef SAFE_MODE
    string msg("DSMATRIX::DSMATRIX(): ");
    if (_n <= 0)
    throw string(msg + "invalid square dimension (> 0).");
#endif // SAFE_MODE
    _is_positive.resize(_n);
    for (int row = 0; row < _n; ++row)
        _is_positive[row].resize(_n); // all zeroes
    assert(check_mat());
}

DSMATRIX::~DSMATRIX() {
}

void DSMATRIX::fill(int const scalar) {
#ifdef SAFE_MODE
    string msg("DSMATRIX::fill(): ");
    if (scalar < 0)
    throw string(msg + "invalid scalar (>= 0).");
#endif // SAFE_MODE            
    if (scalar == 0) {
        _mat *= 0;
        for (int row = 0; row < _n; ++row)
            _is_positive[row].clear();
    } else {
        for (int row = 0; row < _n; ++row)
            for (int col = 0; col < _n; ++col)
                _mat(row, col) = scalar;
        for (int row = 0; row < _n; ++row)
            _is_positive[row].set();
    }
    _row_col_sum = scalar*_n;
    assert(check_mat());
}

int DSMATRIX::get_sum() const {
    return _row_col_sum;
}

void DSMATRIX::add_assignment(ASSIGNMENT *const assigned,
        int const scalar) {
#ifdef SAFE_MODE
    string msg("DSMATRIX::add_assignment(): ");
    if (!assigned)
    throw string(msg + "null pointer to assignment.");
    if (!assigned->is_complete())
    throw string(msg + "assignment incomplete.");
    if (scalar < 0)
    throw string(msg + "invalid scalar (>= 0).");
#endif // SAFE_MODE        
    for (LOCATION loc = 0; loc < _n; ++loc) {
        int const row = static_cast<int>(loc);
        int const col = static_cast<int>(assigned->get_facility(loc));
        _mat(row, col) += scalar;
        _is_positive[row][col] = 1;
    }
    _row_col_sum += scalar;
    assert(check_mat());
}

void DSMATRIX::add_assignments(vector<ASSIGNMENT *> assignments,
        vector<int> scalars) {
#ifdef SAFE_MODE
    string msg("::add_assignments(): ");
    if (assignments.size() != scalars.size())
    throw string(msg + "number of assignments and scalars don't match.");
#endif // SAFE_MODE
    int const num_assignments = assignments.size();
    for (int i = 0; i < num_assignments; ++i)
        add_assignment(assignments[i], scalars[i]);
}

DSMATRIX & DSMATRIX::operator*=(int const scalar) {
#ifdef SAFE_MODE
    string msg("DSMATRIX::operator*=(): ");
    if (scalar < 0)
    throw string(msg + "invalid scaling factor.");
#endif // SAFE_MODE
    if (scalar == 0)
    fill(0);
    else if (scalar> 1) {
        _mat *= scalar;
        _row_col_sum *= scalar;
        // no change to _is_positive matrix
        assert(check_mat());
    }
    return *this;
}

DSMATRIX & DSMATRIX::operator+=(DSMATRIX const& rhs) {
    if (rhs._row_col_sum != 0) {
        this->_mat += rhs._mat;
        this->_row_col_sum += rhs._row_col_sum;
        for (int row = 0; row < _n; ++row)
        this->_is_positive[row] |= rhs._is_positive[row];
        assert(check_mat());
    }
    return *this;
}

ASSIGNMENT * DSMATRIX::remove_assignment(PRNG & prng) {
    if (_row_col_sum == 0)
        return 0;
    ASSIGNMENT *const assigned = new ASSIGNMENT(_n);
    assert(assigned);
    dynamic_bitset<> is_unpicked(_n);
    is_unpicked.set();
    for (int row = 0; row < _n; ++row) {
        // for each row randomly pick a positive entry whose column
        // has not been previously picked
        dynamic_bitset<> is_avail(_is_positive[row] & is_unpicked);
        int const avail_cnt = is_avail.count();
        // get a random integer between 0 and avail_cnt-1 inclusive
        assert(avail_cnt > 0);
        int const rnd = prng(avail_cnt);
        // count rnd (skipping over unavailable entries) positions to
        // select a column for assignment
        int sel_col = is_avail.find_first();
        for (int i = 0; i < rnd; ++i)
            sel_col = is_avail.find_next(sel_col);
        cout << sel_col << " ";
        // update _mat: decrement (row, sel_col), check if resulting
        // entry is less than one and make sel_col a selected column
        _is_positive[row][sel_col] = !!(--_mat(row, sel_col));
        is_unpicked[sel_col] = 0;
        assigned->set_facility(row, sel_col);
    }
    cout << endl;
    
    for (int row = 0; row < _n; ++row){
        for (int col = 0; col < _n; ++col)
            cout << _mat(row, col) << " ";
        cout << endl;
    }
    cout << endl;
    
    assigned->set_complete();
    --_row_col_sum;
    assert(check_mat());
    return assigned;
}

void DSMATRIX::remove_assignments(vector<ASSIGNMENT *> assignments,
        int const num_assignments, PRNG & prng) {
    assignments.resize(num_assignments + assignments.size());
    for (int i = 0; i < num_assignments; ++i) {
        cout << "remove_assignments1 " << i << endl;
        assignments[i] = remove_assignment(prng);
        cout << "remove_assignments2 " << i << endl;
    }
}
