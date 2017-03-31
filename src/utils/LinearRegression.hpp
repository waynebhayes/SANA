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
    int SIZE;
    map<double, double> chart;
    vector<string> temperatures;
    LinearRegression();
    void setup(map<double, double> graph);
    tuple<int, double, double, int, double, double, double, double> run();
    double incrementalValues(int oldIndex1, int oldIndex2, bool index1Change, double sum, vector<double> &data);
    double* incrementalValues(int oldIndex1, int oldIndex2, bool index1Change, double values[], vector<double> &data);
    double linearLeastSquares(double sum, int n);
    double* linearLeastSquares(double values[], int n);
    double leastSquaresError(int index1, int index2, double constant, vector<double> &data);
    double leastSquaresError(int index1, int index2, double constants[], vector<double> &data);
    double* initialValues(int index1, int index2, vector<double> &data);
    double initialSum(int index1, int index2, vector<double> &data);
};
