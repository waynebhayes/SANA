#include "NormalDistribution.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <random>
#include <cmath>
#include "utils.hpp"
using namespace std;

NormalDistribution::NormalDistribution(const vector<double>& samples) {
    mean = vectorMean(samples);
    uint N = samples.size();
    double m = 0;
    for (uint i = 0; i < N; i++) {
        m += (mean - samples[i])*(mean - samples[i]);
    }
    sd = sqrt(1./N * m);
}

NormalDistribution::~NormalDistribution() {

}

double NormalDistribution::getMean() {
    return mean;
}

double NormalDistribution::getSD() {
    return sd;
}

double NormalDistribution::Psmaller(double value) {
    return 1./2 * (1 + erf((value-mean)/(sd*sqrt(2))));
}

double NormalDistribution::Plarger(double value) {
    return 1 - Psmaller(value);
}


const double Pi = atan(1.)*4;
double c;

/*  Erf(x) */
static const int ncof=28;

const double cof[28] = {-1.3026537197817094, 6.4196979235649026e-1,
    1.9476473204185836e-2,-9.561514786808631e-3,-9.46595344482036e-4,
    3.66839497852761e-4,4.2523324806907e-5,-2.0278578112534e-5,
    -1.624290004647e-6,1.303655835580e-6,1.5626441722e-8,-8.5238095915e-8,
    6.529054439e-9,5.059343495e-9,-9.91364156e-10,-2.27365122e-10,
    9.6467911e-11, 2.394038e-12,-6.886027e-12,8.94487e-13, 3.13092e-13,
    -1.12708e-13,3.81e-16,7.106e-15,-1.523e-15,-9.4e-17,1.21e-16,-2.8e-17};
double erfccheb(double z){
    int j;
    double t,ty,tmp,d=0.,dd=0.;
    if (z < 0.) throw("erfccheb requereix un argument no negatiu");
    t = 2./(2.+z);
    ty = 4.*t - 2.;
    for (j=ncof-1;j>0;j--) {
        tmp = d;
        d = ty*d - dd + cof[j];
        dd = tmp;
    }
    return t*exp(-z*z + 0.5*(cof[0] + ty*d) - dd);
}
inline double Erf(double x) {
    if (x >=0.) return 1.0 - erfccheb(x);
    else return erfccheb(-x) - 1.0;
}

/*  Bolzano finder */
const double valors_erf[13] = {
    0.000000000000000, 0.520499877813046, 0.842700792949714, 0.966105146475310, 0.995322265018952,
    0.999593047982555, 0.999977909503001, 0.999999256901627, 0.999999984582742, 0.999999999803383,
    0.999999999998462, 0.999999999999992, 1.000000000000000};
void IntervalBolzano(double c, double &a, double &b) {
    int idx;
    for (idx = 0; idx < 12; idx++) {
        if (valors_erf[idx] <= c and valors_erf[idx+1] >= c) // erf es funcio creixent
            break;
    }
    a = idx * 0.5; b = a + 0.5;
}

/*  Equation erf(x)-c = 0 */
double erf_despl(double x) {
    return Erf(x) - c;
}

/*  Erf derivative */
double derf(double x) {
    return 2 * exp(-x*x) / sqrt(Pi);
}

/*  Newton-Raphson method for finding roots */
double znewton(double fun(double), double dfun(double), double x0, double tol, int& iters) {
    double x;
    double err;
    iters = 0;
    do {
        x = x0 - fun(x0)/dfun(x0); // assumeix dfun(x0) != 0
        err = abs(x-x0);
        x0 = x;
        ++iters;
    } while ( err >= tol );
    return x;
}

/*  Erf inv */
double erfinv(double x) {
    double a, b;
    c = abs(x); // c es global
    IntervalBolzano(c, a, b);

    double tol = 0.5e-15;

    int iters;
    double res = znewton(erf_despl, derf, (a+b)/2, tol, iters);
    return (x > 0.) ? res : -res;
}


double NormalDistribution::quantile(double p) {
    return mean + sd*sqrt(2)*erfinv(2*p-1);
}
