#ifndef NORMALDISTRIBUTION_HPP
#define NORMALDISTRIBUTION_HPP
#include <string>
#include <iostream>
#include <vector>
#include "utils.hpp"

class NormalDistribution {
public:
    NormalDistribution(const vector<double>& samples);
    ~NormalDistribution();

    double getMean();
    double getSD();
    double Psmaller(double value);
    double Plarger(double value);
    double quantile(double p);

private:
    double mean;
    double sd;
};

#endif

