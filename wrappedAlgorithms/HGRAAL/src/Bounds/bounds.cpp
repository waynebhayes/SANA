// standard
#include <list>
using std::list;
#include <cmath>
// local
#include "../Hungarian/munkres.h"
#include "glb.h"
#include "bounds.h"

double rearrangement_bound(vector<double> const& vec1,
        vector<double> const& vec2, bool is_lower) {
    assert(vec1.size() != 0);
    assert(vec1.size() == vec2.size());

    vector<double> tmp1(vec1), tmp2(vec2);
    int const n = vec1.size();

    // sort vectors
    sort(tmp1.begin(), tmp1.end());
    sort(tmp2.begin(), tmp2.end());

    // compute bounds
    double sum = 0.0;
    if (is_lower) // lower bound
        for (int i = 0; i < n; ++i)
            sum += tmp1[i] * tmp2[n-i-1];
    else
        // upper bound
        for (int i = 0; i < n; ++i)
            sum += tmp1[i] * tmp2[i];
    return sum;
}

double full_glb(QAP * problem, bool const is_lower) {
    int const n = problem->get_size();
    Matrix<double> null(n, n);
    null.clear();
    GLB bound(problem, null, is_lower);
    vector<int> new_asgnd(n);
    new_asgnd.clear();
    return bound.solve(new_asgnd);
}

double partial_glb(QAP * problem, bool const is_lower,
        vector<int> asgnd_locs, vector<int> asgnd_facs) {
    int const n = problem->get_size();
    int const npartial = asgnd_locs.size();

    // evaluate QAP cost function for partial assignments
    double asgnd_cost = 0.0;
    for (int i = 0; i < npartial; ++i)
        for (int j = 0; j < npartial; ++j)
            if (i != j)
                asgnd_cost += problem->get_distance(asgnd_locs[i],
                        asgnd_locs[j]) * problem->get_flow(asgnd_facs[i],
                        asgnd_facs[j]);

    // compute cost due to each new facility-loc pairing in
    // relation to existing pairs
    int const unasgnd_n = n - npartial;
    list<int> unasgnd_locs_list, unasgnd_facs_list;
    for (int i = 0; i < n; ++i) {
        unasgnd_locs_list.push_back(i);
        unasgnd_facs_list.push_back(i);
    }
    for (int i = 0; i < npartial; ++i) {
        unasgnd_locs_list.remove(asgnd_locs[i]);
        unasgnd_facs_list.remove(asgnd_facs[i]);
    }

    list<int>::iterator it;
    vector<int> unasgnd_locs;
    for (it = unasgnd_locs_list.begin(); it != unasgnd_locs_list.end(); ++it)
        unasgnd_locs.push_back(*it);

    vector<int> unasgnd_facs;
    for (it = unasgnd_facs_list.begin(); it != unasgnd_facs_list.end(); ++it)
        unasgnd_facs.push_back(*it);

    Matrix<DISTANCE> unasgnd_dmat(unasgnd_n, unasgnd_n);
    Matrix<FLOW> unasgnd_fmat(unasgnd_n, unasgnd_n);
    for (int row = 0; row < unasgnd_n; ++row)
        for (int col = 0; col < unasgnd_n; ++col) {
            unasgnd_dmat.set(row, col, problem->get_distance(
                    unasgnd_locs[row], unasgnd_locs[col]));
            unasgnd_fmat.set(row, col, problem->get_flow(unasgnd_facs[row],
                    unasgnd_facs[col]));
        }

    Matrix<double> cmat(unasgnd_n, unasgnd_n);
    for (int i = 0; i < unasgnd_n; ++i)
        for (int j = 0; j < unasgnd_n; ++j) {
            double tmp = asgnd_cost;
            for (int k = 0; k < npartial; ++k)
                tmp += problem->get_distance(unasgnd_locs[i], asgnd_locs[k])
                        * problem->get_flow(unasgnd_facs[j], asgnd_facs[k]);
            cmat(i, j) = tmp;
        }

    QAP * sub_problem = new QAP(unasgnd_dmat, unasgnd_fmat);
    GLB bound(sub_problem, cmat, is_lower);
    vector<int> new_asgnd(unasgnd_n);
    new_asgnd.clear();
    return bound.solve(new_asgnd);
}

double two_level_glb(QAP * problem, bool const is_lower) {
    int const n = problem->get_size();
    if (is_lower) {
        double min_cost = 0.0;
        for (int i = 0; i < n; ++i) {
            vector<int> asgnd_locs;
            asgnd_locs.clear();
            asgnd_locs.push_back(i);
            for (int j = 0; j < n; ++j) {
                vector<int> asgnd_facs;
                asgnd_facs.clear();
                asgnd_facs.push_back(j);
                double const cost = partial_glb(problem, is_lower,
                        asgnd_locs, asgnd_facs);
                if ((cost < min_cost) || ((i == 0) && (j == 0)))
                    min_cost = cost;
                cout.setf(ios_base::fixed, ios_base::floatfield);
                cout << "(" << i << "," << j << ") : " << cost << endl;
            }
        }
        return min_cost;
    } else {
        double max_cost = 0.0;
        for (int i = 0; i < n; ++i) {
            vector<int> asgnd_locs;
            asgnd_locs.clear();
            asgnd_locs.push_back(i);
            for (int j = 0; j < n; ++j) {
                vector<int> asgnd_facs;
                asgnd_facs.clear();
                asgnd_facs.push_back(j);
                double const cost = partial_glb(problem, is_lower,
                        asgnd_locs, asgnd_facs);
                if (cost > max_cost)
                    max_cost = cost;
            }
        }
        return max_cost;
    }
}

template <class T> void sort_matrix(Matrix<T> & m) {
    size_t n = m.columns();

    // replace diagonal entries with end-of-row
    // entries to make contiguous rows
    // last row already contiguous
    for (QAP_INDEX row = 0; row < (n-1); ++row) {
        QAP_INDEX const col = row;
        m.set(row, col, m.get(row, n-1));
    }

    // sort rows O(n^2 lg n)-time
    vector<QAP_VALUE> tmp_row(n-1);
    for (QAP_INDEX row = 0; row < n; ++row) {
        for (QAP_INDEX col = 0; col < (n-1); ++col)
            tmp_row[col] = m.get(row, col);
        sort(tmp_row.begin(), tmp_row.end());
        for (QAP_INDEX col = 0; col < (n-1); ++col)
            m.set(row, col, tmp_row[col]);
    }
}

Matrix<COST> & compute_local_glbs(QAP *const problem,
        bool const is_lower) {
    assert(problem != 0);

    size_t n = problem->get_size();

    // sort rows for distance matrix
    Matrix<DISTANCE> sorted_dmat(n, n);
    for (LOCATION src = 0; src < n; ++src)
        for (LOCATION dst = 0; dst < n; ++dst)
            sorted_dmat.set(src, dst, problem->get_distance(src, dst));
    sort_matrix<DISTANCE>(sorted_dmat);

    // sort rows for flow matrix
    Matrix<FLOW> sorted_fmat(n, n);
    for (FACILITY src = 0; src < n; ++src)
        for (FACILITY dst = 0; dst < n; ++dst)
            sorted_fmat.set(src, dst, problem->get_flow(src, dst));
    sort_matrix<FLOW>(sorted_fmat);

    // compute rearrangement bound for each source location-facility pair
    Matrix<COST> * cmat = new Matrix<COST>(n, n);
    for (LOCATION src_loc = 0; src_loc < n; ++src_loc)
        for (FACILITY src_fac = 0; src_fac < n; ++src_fac) {
            COST cost = 0.0;
            if (is_lower) // lower bound
                for (QAP_INDEX dst = 0; dst < (n-1); ++dst)
                    cost
                            += sorted_dmat.get(src_loc, static_cast<LOCATION>(dst))
                                    * sorted_fmat.get(src_fac, static_cast<FACILITY>(n
                                            -dst-2));
            else
                // upper bound
                for (QAP_INDEX dst = 0; dst < (n-1); ++dst)
                    cost
                            += sorted_dmat.get(src_loc, static_cast<LOCATION>(dst))
                                    * sorted_fmat.get(src_fac,
                                            static_cast<FACILITY>(dst));
            cmat->set(static_cast<int>(src_loc), static_cast<int>(src_fac),
                    cost);
        }
    return *cmat;
}

COST compute_lap_cost(Matrix<COST> & cmat, bool const is_lower) {
    size_t n = cmat.columns();
    Matrix<COST> ori_cmat(cmat);

    if (!is_lower) {
        // negate and add maximum cost
        COST max = 0.0;
        for (LOCATION loc = 0; loc < n; ++loc)
            for (FACILITY fac = 0; fac < n; ++fac) {
                COST const tmp = cmat.get(loc, fac);
                if (tmp > max)
                    max = tmp;
            }
        for (LOCATION loc = 0; loc < n; ++loc)
            for (FACILITY fac = 0; fac < n; ++fac)
                cmat.set(loc, fac, max - cmat.get(loc, fac));
    }

    static Munkres m;
    m.solve(cmat); // O(n^3)-time

    COST cost = 0.0;
    for (LOCATION loc = 0; loc < n; ++loc)
        for (FACILITY fac = 0; fac < n; ++fac)
            if (cmat.get(loc, fac) == 0.0)
                cost += ori_cmat(loc, fac);

    return cost;
}

void compute_local_labs(Matrix<COST> & cmat, QAP *const problem,
        double const d, bool const is_lower) {
    assert(problem != 0);

    size_t n = problem->get_size();
    Matrix<COST> ori_cmat(cmat);

    // compute linear assignment bound for each source location-facility pair
    for (LOCATION src_loc = 0; src_loc < n; ++src_loc)
        for (FACILITY src_fac = 0; src_fac < n; ++src_fac) {
            // create submatrix with src_loc row and src_fac column removed
            Matrix<COST> sub_cmat(n-1, n-1);
            LOCATION const loc_ends[2][2] = { { 0, src_loc }, { src_loc+1,
                    n } };
            FACILITY const fac_ends[2][2] = { { 0, src_fac }, { src_fac+1,
                    n } };
            for (int loc_seg = 0; loc_seg < 1; ++loc_seg)
                for (int fac_seg = 0; fac_seg < 1; ++fac_seg)
                    for (LOCATION dst_loc = loc_ends[loc_seg][0]; dst_loc
                            < loc_ends[loc_seg][1]; ++dst_loc)
                        for (FACILITY dst_fac = fac_ends[fac_seg][0]; dst_fac
                                < fac_ends[fac_seg][1]; ++dst_fac) {
                            COST const product = problem->get_distance(src_loc,
                                    dst_loc) * problem->get_flow(src_fac, dst_fac);
                            COST const update = ((1-d) * ori_cmat.get(dst_loc,
                                    dst_fac)) + (d * product);
                            sub_cmat.set(dst_loc-loc_seg, dst_fac-fac_seg, update);
                        }
            cmat.set(src_loc, src_fac, compute_lap_cost(sub_cmat, is_lower));
        }
}

// k = 1 means the usual GLB
double tree_glb(QAP * problem, int const k, double const d,
        bool const is_lower) {
    // initialize cmat with local GLBs
    Matrix<COST> cmat = compute_local_glbs(problem, is_lower);
    for (int i = 0; i < k-1; ++i)
        compute_local_labs(cmat, problem, d, is_lower);
    COST cost = compute_lap_cost(cmat, is_lower);
    // normalize cost
    size_t const n = problem->get_size();
    COST const r = (1-d)*(n-1);
    COST const p = pow(r, k-1);
    COST const divisor = d*(p-1)/(r-1) + p;
    return cost/divisor;
}
