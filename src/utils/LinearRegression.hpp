#ifndef LINEARREGRESSION_HPP
#define LINEARREGRESSION_HPP
//
// Created by Hudson Hughes on 8/2/16.
#include "../arguments/ArgumentParser.hpp"
#include "../arguments/supportedArguments.hpp"
#include "../arguments/defaultArguments.hpp"
#include "../arguments/modeSelector.hpp"
#include "randomSeed.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include "utils.hpp"
using namespace std;

class LinearRegression {

public:

    LinearRegression(multimap<double, double> chart, bool convertXToLogScale);
    LinearRegression(map<double, double> chart);

    tuple<int, double, double, int, double, double, double, double> run() const;

private:
    vector<double> xs;
    vector<double> ys;

    static double incrementalValues(int oldIndex1, int oldIndex2, bool index1Change, double sum, const vector<double> &data);
    static double* incrementalValues(int oldIndex1, int oldIndex2, bool index1Change, double values[], const vector<double> &data);
    static double linearLeastSquares(double sum, int n);
    static double* linearLeastSquares(double values[], int n);
    static double leastSquaresError(int index1, int index2, double constant, const vector<double> &data);
    static double leastSquaresError(int index1, int index2, double constants[], const vector<double> &data);
    static double* initialValues(int index1, int index2, const vector<double> &data);
    static double initialSum(int index1, int index2, const vector<double> &data);
};


#endif