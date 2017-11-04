/*
 *   Copyright (c) 2007 John Weaver
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include "munkres.h"
#include "algorithm"
using std::sort;
using std::unique;

#define Z_NORMAL 0
#define Z_STAR 1
#define Z_PRIME 2

void Munkres::countzeros(string const s) {
    int cnt = 0;
    for (int row = 0; row < nrows; ++row)
        for (int col = 0; col < ncols; ++col)
            cnt += (matrix(row, col) == 0);
    cout << "zero-count at " << s << " = " << cnt << endl;
}

bool Munkres::pair_in_list(const std::pair<int,int> &needle,
        const std::list<std::pair<int,int> > &haystack) {
    for (std::list<std::pair<int,int> >::const_iterator i =
            haystack.begin(); i != haystack.end(); ++i)
        if (needle == *i)
            return true;
    return false;
}

// Find a zero (Z) in the resulting matrix.  If there is no starred
// zero in its row or column, star Z. Repeat for each element in the
// matrix. Go to Step 3.
// Cover each column containing a starred zero.  If K columns are
// covered, the starred zeros describe a complete set of unique
// assignments.  In this case, Go to DONE, otherwise, Go to Step 4.

// notes: mask_matrix can be implemented as bit vectors
int Munkres::step12() {
    countzeros("begin");

    // subtract min value from each row
    for (int row = 0; row < nrows; ++row) {
        double h = matrix(row, 0);
        for (int col = 1; col < ncols; ++col) {
            double const tmp = matrix(row, col);
            if (tmp < h)
                h = tmp;
            if (h == 0)
                break;
        }
        if (h > 0)
            for (int col = 0; col < ncols; ++col)
                matrix(row, col) -= h;
    }

    // subtract min value from each column
    for (int col = 0; col < ncols; ++col) {
        double h = matrix(0, col);
        for (int row = 1; row < nrows; ++row) {
            double const tmp = matrix(row, col);
            if (tmp < h)
                h = tmp;
            if (h == 0)
                break;
        }
        if (h > 0)
            for (int row = 0; row < nrows; ++row)
                matrix(row, col) -= h;
    }

    // clear row and column covers
    for (int row = 0; row < nrows; ++row)
        row_mask[row] = false;
    for (int col = 0; col < ncols; ++col)
        col_mask[col] = false;

    // star zero found on uncovered row and column
    for (int row = 0; row < nrows; ++row) {
        if (row_mask[row])
            continue;
        for (int col = 0; col < ncols; ++col)
            if ((!col_mask[col]) && (matrix(row, col) == 0)) {
                mask_matrix(row, col) = Z_STAR;
                col_mask[col] = row_mask[row] = true;
                break;
            }
    }
    countzeros("end");
    return 3;
}

int Munkres::step3() {
    // clear row and column covers
    for (int row = 0; row < nrows; ++row)
        row_mask[row] = false;
    for (int col = 0; col < ncols; ++col)
        col_mask[col] = false;

    // count column covers
    int covercount = 0;
    for (int col = 0; col < ncols; ++col)
        for (int row = 0; row < nrows; ++row)
            if (mask_matrix(row, col) == Z_STAR) {
                col_mask[col] = true;
                ++covercount;
                break;
            }
    cout << string("\r") + "column cover count = " << covercount;
    return (covercount >= nrows) ? 0 : 4;
}

// Find a noncovered zero and prime it.  If there is no starred zero
// in the row containing this primed zero, Go to Step 5.  Otherwise,
// cover this row and uncover the column containing the starred zero.
// Continue in this manner until there are no uncovered zeros left.
// Save the smallest uncovered value and Go to Step 6.
int Munkres::step4() {
    /*
     Main Zero Search

     1. Find an uncovered Z in the distance matrix and prime it. If no
     such zero exists, go to Step 5
     2. If No Z* exists in the row of the Z', go to Step 4.
     3. If a Z* exists, cover this row and uncover the column of the Z*.
     Return to Step 3.1 to find a new Z
     */
    for (savecol = 0; savecol < ncols; ++savecol) {
        if (col_mask[savecol])
            continue;
        for (saverow = 0; saverow < nrows; ++saverow)
            if ((!row_mask[saverow]) && (matrix(saverow, savecol) == 0))
                goto found_uncovered_zero;
    }
    return 6;

    found_uncovered_zero:

    mask_matrix(saverow, savecol) = Z_PRIME; // prime it.
    for (int col = 0; col < ncols; ++col)
        if (mask_matrix(saverow, col) == Z_STAR) {
            row_mask[saverow] = true; //cover this row and
            col_mask[col] = false; // uncover the column containing the starred zero
            return 4; // repeat
        }

    return 5; // no starred zero in the row containing this primed zero
}

int Munkres::step5() {
    std::list<std::pair<int,int> > seq;
    // use saverow, savecol from step 3.
    std::pair<int,int> z0(saverow, savecol);
    std::pair<int,int> z1(-1, -1);
    std::pair<int,int> z2n(-1, -1);
    seq.insert(seq.end(), z0);
    int row, col = savecol;
    /*
     Increment Set of Starred Zeros

     1. Construct the ``alternating sequence'' of primed and starred zeros:

     Z0 : Unpaired Z' from Step 4.2
     Z1 : The Z* in the column of Z0
     Z[2N] : The Z' in the row of Z[2N-1], if such a zero exists
     Z[2N+1] : The Z* in the column of Z[2N]

     The sequence eventually terminates with an unpaired Z' = Z[2N] for some N.
     */
    for (;;) {
        // check column for starred zeroes
        for (row = 0; row < nrows; ++row)
            if (mask_matrix(row, col) == Z_STAR) {
                z1.first = row;
                z1.second = col;
                if (pair_in_list(z1, seq))
                    continue;
                seq.insert(seq.end(), z1);
                goto prime;
            }
        break; // could not find starred zeroes

        prime:

        // check row for primed zeroes (guaranteed to find one)
        for (col = 0; col < ncols; ++col)
            if (mask_matrix(row, col) == Z_PRIME) {
                z2n.first = row;
                z2n.second = col;
                if (pair_in_list(z2n, seq))
                    continue;
                seq.insert(seq.end(), z2n);
                break;
            }
    }

    for (std::list<std::pair<int,int> >::iterator i = seq.begin(); i
            != seq.end(); i++) {
        int const mask = mask_matrix(i->first, i->second);
        if (mask == Z_STAR)
            // 2. Unstar each starred zero of the sequence.
            mask_matrix(i->first, i->second) = Z_NORMAL;
        else if (mask == Z_PRIME)
            // 3. Star each primed zero of the sequence,
            // thus increasing the number of starred zeros by one.
            mask_matrix(i->first, i->second) = Z_STAR;
    }

    // 4. Erase all primes
    for (int row = 0; row < nrows; ++row)
        for (int col = 0; col < ncols; ++col)
            if (mask_matrix(row, col) == Z_PRIME)
                mask_matrix(row, col) = Z_NORMAL;

    return 3;
}

int Munkres::step6() {
    /*
     New Zero Manufactures

     1. Let h be the smallest uncovered entry in the (modified) distance matrix.
     2. Add h to all covered rows.
     3. Subtract h from all uncovered columns
     4. Return to Step 3, without altering stars, primes, or covers.
     */

    double h = -1;
    for (int row = 0; row < nrows; ++row) {
        if (row_mask[row])
            continue;
        for (int col = 0; col < ncols; ++col) {
            if (col_mask[col])
                continue;
            double const tmp = matrix(row, col);
            if (h > tmp || h == -1)
                h = tmp;
        }
    }

    for (int row = 0; row < nrows; ++row)
        if (row_mask[row])
            for (int col = 0; col < ncols; ++col)
                matrix(row, col) += h;
    for (int col = 0; col < ncols; ++col)
        if (!col_mask[col])
            for (int row = 0; row < nrows; ++row)
                matrix(row, col) -= h;

    return 4;
}

void Munkres::solve(Matrix<double> & m) {
    // Linear assignment problem solution
    // [modifies matrix in-place.]
    // matrix(row,col): row major format assumed.

    // Assignments are remaining 0 values
    // (extra 0 values are replaced with -1)

    this->matrix = m;
    nrows = matrix.rows(), ncols = matrix.columns();
    assert(nrows <= ncols);
    // Z_STAR == 1 == starred, Z_PRIME == 2 == primed
    mask_matrix.resize(nrows, ncols);

    // initialize row and column masks
    row_mask = new bool[nrows];
    col_mask = new bool[ncols];

    for (int step = step12(); step != 0;) {
        switch (step) {
        case 3:
            step = step3();
            break;
        case 4:
            step = step4();
            break;
        case 5:
            step = step5();
            break;
        case 6:
            step = step6();
            break;
        }
    }

    // store results
    for (int row = 0; row < nrows; ++row)
        for (int col = 0; col < ncols; ++col)
            matrix(row, col) = (mask_matrix(row, col) == Z_STAR) ? 0 : -1;

    m = matrix;

    delete [] row_mask;
    delete [] col_mask;
}

void Munkres::simple_solve(vector<int> & sol, Matrix<double> & m) {
    // Linear assignment problem solution
    // [modifies matrix in-place.]
    // matrix(row,col): row major format assumed.

    // Assignments are remaining 0 values
    // (extra 0 values are replaced with -1)

    this->matrix = m;
    nrows = matrix.rows(), ncols = matrix.columns();
    assert(nrows <= ncols);
    // Z_STAR == 1 == starred, Z_PRIME == 2 == primed
    mask_matrix.resize(nrows, ncols);

    // initialize row and column masks
    row_mask = new bool[nrows];
    col_mask = new bool[ncols];

    for (int step = step12(); step != 0;) {
        switch (step) {
        case 3:
            step = step3();
            break;
        case 4:
            step = step4();
            break;
        case 5:
            step = step5();
            break;
        case 6:
            step = step6();
            break;
        }
    }

    // store results
    for (int row = 0; row < nrows; ++row)
        for (int col = 0; col < ncols; ++col)
            if (mask_matrix(row, col) == Z_STAR) {
                sol[row] = col;
                break;
            }

    m = matrix;

    delete [] row_mask;
    delete [] col_mask;
}

void Munkres::find_optimal_edge_set(vector<set<int> > & opt_edges,
        vector<int> & sol, Matrix<double> & m, Matrix<double> & ori_m,
        int const start_row, int const end_row) {
    // Linear assignment problem solution
    // [modifies matrix in-place.]
    // matrix(row,col): row major format assumed.

    // Assignments are remaining 0 values
    // (extra 0 values are replaced with -1)

    this->matrix = m;
    nrows = matrix.rows(), ncols = matrix.columns();
    assert(nrows <= ncols);
    // Z_STAR == 1 == starred, Z_PRIME == 2 == primed
    mask_matrix.resize(nrows, ncols);

    // initialize row and column masks
    row_mask = new bool[nrows];
    col_mask = new bool[ncols];

    // reconstitute mask_matrix from sol
    double sum = 0.0;
    for (int row = 0; row < nrows; ++row) {
        for (int col = 0; col < ncols; ++col)
            mask_matrix(row, col) = Z_NORMAL;
        mask_matrix(row, sol[row]) = Z_STAR;
        sum += ori_m(row, sol[row]);
    }

    double const INF = 1000000;

    for (int row = 0; row <= (end_row-start_row); ++row) {
        opt_edges.push_back(*(new set<int>));
        opt_edges[row].insert(sol[row+start_row]);
    }

    Matrix<int> tmp_mask_matrix = mask_matrix;
    Matrix<double> tmp_matrix = matrix;
    int max_row_cnt = 0;
    for (int row = 0; row <= (end_row-start_row); ++row) { // outer loop
        for (;;) { // inner loop
            // unstar and perturb edges
            mask_matrix(row+start_row, sol[row+start_row]) = Z_NORMAL;
            for (set<int>::iterator it = opt_edges[row].begin(); it
                    != opt_edges[row].end(); ++it)
                matrix(row+start_row, *it) = INF;

            for (int step = 3; step != 0;) {
                switch (step) {
                case 3:
                    step = step3();
                    break;
                case 4:
                    step = step4();
                    break;
                case 5:
                    step = step5();
                    break;
                case 6:
                    step = step6();
                    break;
                }
            }

            // store results
            double opt_sum = 0.0;
            vector<int> tmp_sol(nrows);
            for (int j = 0; j < nrows; ++j)
                for (int col = 0; col < ncols; ++col)
                    if (mask_matrix(j, col) == Z_STAR) {
                        tmp_sol[j] = col;
                        opt_sum += ori_m(j, tmp_sol[j]);
                        break;
                    }

            if (sum == opt_sum) {
                // update opt_edges
                for (int j = row; j <= (end_row-start_row); ++j)
                    opt_edges[j].insert(tmp_sol[j+start_row]);

                // restore original algorithm data structures
                mask_matrix = tmp_mask_matrix;
                matrix = tmp_matrix;

                continue; // continue with inner loop
            } else {
                // restore original algorithm data structures
                mask_matrix = tmp_mask_matrix;
                matrix = tmp_matrix;

                break; // continue with outer loop
            }
        }
        if (max_row_cnt < opt_edges[row].size())
            max_row_cnt = opt_edges[row].size();
        cout << "row = " << row << " " << "row count = "
                << opt_edges[row].size() << "(" << max_row_cnt << ")" << endl;
    }

    delete [] row_mask;
    delete [] col_mask;
}

void Munkres::multi_solve(vector<vector<int> > & sols,
        vector<vector<int> > & opt_edges, Matrix<double> & m) {
    // Linear assignment problem solution
    // [modifies matrix in-place.]
    // matrix(row,col): row major format assumed.

    // Assignments are remaining 0 values
    // (extra 0 values are replaced with -1)

    this->matrix = m;
    nrows = matrix.rows(), ncols = matrix.columns();
    assert(nrows <= ncols);
    // Z_STAR == 1 == starred, Z_PRIME == 2 == primed
    mask_matrix.resize(nrows, ncols);

    // initialize row and column masks
    row_mask = new bool[nrows];
    col_mask = new bool[ncols];

    for (int step = step12(); step != 0;) {
        switch (step) {
        case 3:
            step = step3();
            break;
        case 4:
            step = step4();
            break;
        case 5:
            step = step5();
            break;
        case 6:
            step = step6();
            break;
        }
    }

    // store results
    double sum = 0.0;
    vector<int> * sol = new vector<int>(nrows);
    for (int row = 0; row < nrows; ++row)
        for (int col = 0; col < ncols; ++col)
            if (mask_matrix(row, col) == Z_STAR) {
                (*sol)[row] = col;
                sum += m(row, (*sol)[row]);
                break;
            }

    sols.push_back(*sol);

    double const INF = 1000000;

    for (int row = 0; row < nrows; ++row) {
        opt_edges.push_back(*(new vector<int>));
        opt_edges[row].push_back(sols[0][row]);
    }

    Matrix<int> tmp_mask_matrix = mask_matrix;
    Matrix<double> tmp_matrix = matrix;
    for (int row = 0; row < nrows; ++row) { // outer loop
        for (;;) { // inner loop
            // unstar and perturb edges
            mask_matrix(row, opt_edges[row][0]) = Z_NORMAL;
            for (int k = 0; k < opt_edges[row].size(); ++k)
                matrix(row, opt_edges[row][k]) = INF;

            for (int step = 3; step != 0;) {
                switch (step) {
                case 3:
                    step = step3();
                    break;
                case 4:
                    step = step4();
                    break;
                case 5:
                    step = step5();
                    break;
                case 6:
                    step = step6();
                    break;
                }
            }

            // store results
            double opt_sum = 0.0;
            vector<int> * sol = new vector<int>(nrows);
            for (int j = 0; j < nrows; ++j)
                for (int col = 0; col < ncols; ++col)
                    if (mask_matrix(j, col) == Z_STAR) {
                        (*sol)[j] = col;
                        opt_sum += m(j, (*sol)[j]);
                        break;
                    }

            if (sum == opt_sum) {
                // update opt_edges
                for (int j = row; j < nrows; ++j)
                    opt_edges[j].push_back((*sol)[j]);

                sols.push_back(*sol);
                cout << "Found new solution! " << sols.size() << endl;

                // restore original algorithm data structures
                mask_matrix = tmp_mask_matrix;
                matrix = tmp_matrix;

                continue; // continue with inner loop
            } else {
                delete sol;

                // restore original algorithm data structures
                mask_matrix = tmp_mask_matrix;
                matrix = tmp_matrix;

                break; // continue with outer loop
            }
        }
        sort(opt_edges[row].begin(), opt_edges[row].end());
        vector<int>::iterator dup = unique(opt_edges[row].begin(),
                opt_edges[row].end());
        opt_edges[row].erase(dup, opt_edges[row].end());
        cout << "row = " << row << " " << "row count = "
                << opt_edges[row].size() << endl;
    }

    delete [] row_mask;
    delete [] col_mask;
}

#if 0
void Munkres::multi_solve(vector<vector<int> > & sols,
        vector<vector<int> > & opt_edges, Matrix<double> & m) {
    // Linear assignment problem solution
    // [modifies matrix in-place.]
    // matrix(row,col): row major format assumed.

    // Assignments are remaining 0 values
    // (extra 0 values are replaced with -1)

    this->matrix = m;
    nrows = matrix.rows(), ncols = matrix.columns();
    assert(nrows <= ncols);
    // Z_STAR == 1 == starred, Z_PRIME == 2 == primed
    mask_matrix.resize(nrows, ncols);

    // initialize row and column masks
    row_mask = new bool[nrows];
    col_mask = new bool[ncols];

    for (int step = step12(); step != 0;) {
        switch (step) {
            case 3:
            step = step3();
            break;
            case 4:
            step = step4();
            break;
            case 5:
            step = step5();
            break;
            case 6:
            step = step6();
            break;
        }
    }

    // store results
    double sum = 0.0;
    vector<int> * sol = new vector<int>(nrows);
    for (int row = 0; row < nrows; ++row)
    for (int col = 0; col < ncols; ++col)
    if (mask_matrix(row, col) == Z_STAR) {
        (*sol)[row] = col;
        sum += m(row, (*sol)[row]);
        break;
    }

    sols.push_back(*sol);

    double const INF = 1000000;

    Matrix<int> tmp_mask_matrix = mask_matrix;
    Matrix<double> tmp_matrix = matrix;
    for (int row = 0; row < nrows; ++row) { // outer loop
        vector<int> * perturbed_edges = new vector<int>;
        perturbed_edges->push_back(sols[0][row]);

        int row_cnt = 1;
        for (;;) { // inner loop
            // unstar and perturb edges
            mask_matrix(row, (*perturbed_edges)[0]) = Z_NORMAL;
            for (int k = 0; k < perturbed_edges->size(); ++k)
            matrix(row, (*perturbed_edges)[k]) = INF;

            for (int step = 3; step != 0;) {
                switch (step) {
                    case 3:
                    step = step3();
                    break;
                    case 4:
                    step = step4();
                    break;
                    case 5:
                    step = step5();
                    break;
                    case 6:
                    step = step6();
                    break;
                }
            }

            // store results
            double opt_sum = 0.0;
            vector<int> * sol = new vector<int>(nrows);
            for (int j = 0; j < nrows; ++j)
            for (int col = 0; col < ncols; ++col)
            if (mask_matrix(j, col) == Z_STAR) {
                (*sol)[j] = col;
                opt_sum += m(j, (*sol)[j]);
                break;
            }

            if (sum == opt_sum) {
                perturbed_edges->push_back((*sol)[row]);

                // check for duplicates
                bool duplicate = false;
                for (int i = 0; i < sols.size(); ++i) {
                    bool mismatch = false;
                    for (int k = 0; k < nrows; ++k)
                    if ((*sol)[k] != sols[i][k]) {
                        mismatch = true;
                        break;
                    }
                    if (!mismatch) {
                        delete sol;
                        duplicate = true;
                        break;
                    }
                }
                if (!duplicate) {
                    sols.push_back(*sol);
                    cout << "Found new solution! " << sols.size() << endl;
                }

                // restore original algorithm data structures
                mask_matrix = tmp_mask_matrix;
                matrix = tmp_matrix;

                ++row_cnt;
                continue; // continue with inner loop
            } else {
                delete sol;

                // restore original algorithm data structures
                mask_matrix = tmp_mask_matrix;
                matrix = tmp_matrix;

                break; // continue with outer loop
            }
        }
        cout << "row = " << row << " " << "row count = " << row_cnt
        << endl;
        sort(perturbed_edges->begin(), perturbed_edges->end());
        opt_edges.push_back(*perturbed_edges);
    }

    delete [] row_mask;
    delete [] col_mask;
}
#endif
