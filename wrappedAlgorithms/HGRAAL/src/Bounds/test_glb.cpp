#include "../common.h"
// standard
#include <string>
using std::string;
#include <fstream>
using std::ifstream;
#include <algorithm>
#include <vector>
using std::vector;
// local
#include "../Hungarian/matrix.h"
#include "glb.h"

void test_one_file(string const fname, double const qaplib_glb) {
    string msg("::test_one_file");
    ifstream ifs(fname.c_str());

    QAP * problem;
    try {
        problem = new QAP(fname);
    } catch(...) {
        cout << (msg + "error reading problem.");
        throw;
    }
    int const n = problem->get_size();

    // compute Gilmore-Lawler bound for QAP cost
    Matrix<double> null(n, n);
    null.clear();
    GLB bound(problem, null, true);
    vector<int> new_asgnd(n);
    new_asgnd.clear();
    double min_glb = bound.solve(new_asgnd);

    cout.setf(ios_base::fixed, ios_base::floatfield);
    cout << fname << " : " << min_glb << " (own) ";
    if (min_glb == qaplib_glb)
        cout << "=";
    else if (min_glb < qaplib_glb)
        cout << "<";
    else
        cout << ">";
    cout << " " << qaplib_glb << " (qaplib) " << endl;
    for (int i = 0; i < n; ++i)
        cout << new_asgnd[i] << " ";
    cout << endl;
}

void test_glb() {
    int const num = 8;
    string const flist[num] = { "esc128.dat", "tai35a.dat",
            "tai40a.dat", "tai50a.dat", "tai60a.dat", "tai80a.dat",
            "tai100a.dat", "tai150b.dat" };
    double const glblist[num] = { 2, 1951207, 2492850, 3854359,
            5555095, 10329674, 15824355, 63007151 };
    //test_one_file(flist[2], glblist[2]);
    for (int i = 0; i < num; ++i)
        test_one_file(flist[i], glblist[i]);
}
