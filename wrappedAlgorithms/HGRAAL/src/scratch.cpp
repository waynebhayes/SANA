// use this file for writing throwaway test programs

#if 0

/*
#include <boost/lambda/lambda.hpp>
#include <iostream>
#include <iterator>
#include <algorithm>

void test_boost() {
    using namespace boost::lambda;
    typedef std::istream_iterator<int> in;

    std::for_each(
            in(std::cin), in(), std::cout << (_1 * 3) << " " );
}
//////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sha2.h"

#define BUF_SIZE    16384

void testSHA2() {
    sha512_ctx ctx[1];
    unsigned char buf[BUF_SIZE], hval[SHA512_DIGEST_SIZE];

    buf[0] = 0x8f;
    sha512_begin(ctx);
    sha512_hash(buf, 1, ctx);
    sha512_end(hval, ctx);

    printf("\n");
    for (int i = 0; i < SHA512_DIGEST_SIZE; ++i)
        printf("%02x", hval[i]);
    printf("\n");
}
*/
//////////////////////////////////////////////////////////////////////
#include "common.h"

void optimalBits() {
    for (unsigned sz = 2; sz < 65536; ++sz) {
        unsigned tmp = sz, cnt = 0;
        unsigned bitsum = 0;
        while (tmp) {
            ++cnt;
            bitsum += tmp&0x1;
            tmp >>= 1;
        }
        if (bitsum == 1)
            --cnt;
        unsigned w = 1<<cnt, d = w/sz;
        double min_exp_bits = static_cast<double>(cnt*w)/(d*sz);
        unsigned min_x = cnt;
        for (int x = cnt+1; x <= min_exp_bits; ++x) {
            unsigned w = 1<<x, d = w/sz;
            double const exp_bits = static_cast<double>(x*w)/(d
                    *sz);
            if (exp_bits < min_exp_bits) {
                min_exp_bits = exp_bits;
                min_x = x;
            }
        }
        cout << "sz = " << sz << ", min_x = " << min_x
                << ", min_exp_bits = " << min_exp_bits << endl;
    }
}
//////////////////////////////////////////////////////////////////////
// computes average QAP cost given probability matrix for assignments
#include "common.h"
#include "Hungarian/matrix.h"

double computeAvgQAP(Matrix<double> const& prob,
        Matrix<double> const& dmat, Matrix<double> const& fmat) {
    assert(prob.columns() == prob.rows());
    assert(dmat.columns() == dmat.rows());
    assert(fmat.columns() == fmat.rows());
    assert(prob.columns() == dmat.columns());
    assert(prob.columns() == fmat.columns());

    int const n = prob.columns();
    double avg_global_cost = 0.0;
    for (int loc = 0; loc < n; ++loc) {
        double avg_loc_cost = 0.0;
        for (int fac = 0; fac < n; ++fac) {
            double avg_loc_fac_cost = 0.0;
            for (int sub_loc = 0; sub_loc < n; sub_loc) {
                if (sub_loc == loc)
                    continue;
                double avg_loc_fac_flow = 0.0;
                for (int sub_fac = 0; sub_fac < n; ++sub_fac) {
                    if (sub_fac == fac)
                        continue;
                    avg_loc_fac_flow += prob.get(sub_loc, sub_fac)
                            * fmat.get(fac, sub_fac);
                }
                avg_loc_fac_flow /= 1.0-prob.get(loc, fac);
                avg_loc_fac_cost += avg_loc_fac_flow*dmat.get(loc,
                        sub_loc);
            }
            avg_loc_cost += avg_loc_fac_cost*prob.get(loc, fac);
        }
        avg_global_cost += avg_loc_cost;
    }
    return avg_global_cost;
}
#endif
