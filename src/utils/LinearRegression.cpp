//
// Created by Hudson Hughes on 8/2/16.
//
#include <tuple>
#include <vector>
#include <cmath>
#include <assert.h>
#include "LinearRegression.hpp"
using namespace std;


LinearRegression::LinearRegression(multimap<double, double> chart, bool convertXToLogScale) {
    xs = vector<double> (0);
    ys = vector<double> (0);
    for (auto it = chart.begin(); it != chart.end(); it++) {
        double x = it->first;
        xs.push_back(convertXToLogScale ? log(x) : x);
        ys.push_back(it->second);
    }
}

LinearRegression::LinearRegression(map<double, double> chart) {
    xs = vector<double> (0);
    ys = vector<double> (0);
    for (auto it = chart.begin(); it != chart.end(); it++) {
        xs.push_back(it->first);
        ys.push_back(it->second);
    }
}


tuple<int, double, double, int, double, double, double, double> LinearRegression::run() const {
    int SIZE = xs.size();

    double line1Sum;
    double* line2Values;
    double line3Sum;
    double line1LeastSquares;
    double* line2LeastSquares;
    double line3LeastSquares;
    double line1Error;
    double line2Error;
    double line3Error;

    double smallestError = -1;
    int bestJ = -1, bestK = -1;
    double line1Height;
    //double* line2Coeff;
    double line3Height;
    double currentError;

    for (int j = 1; j < SIZE - 2; j++) {
        line1Sum = initialSum(0, j, ys);
        line2Values = initialValues(j, j+1, ys);
        line3Sum = initialSum(j+1, SIZE - 1, ys);

        line1LeastSquares = linearLeastSquares(line1Sum, j+1);
        line1Error = leastSquaresError(0, j, line1LeastSquares, ys);

        for (int k = j+1; k < SIZE - 1; k++) {
            line2LeastSquares = linearLeastSquares(line2Values, k-j+1);
            line3LeastSquares = linearLeastSquares(line3Sum, SIZE-k);

            line2Error = leastSquaresError(j, k, line2LeastSquares, ys);
            line3Error = leastSquaresError(k, SIZE - 1, line3LeastSquares, ys);

            currentError = line1Error + line2Error + line3Error;

            if (j == 1 && k == 2) {
                smallestError = currentError;
                bestJ = j;
                bestK = k;
                line1Height = line1LeastSquares;
                // line2Coeff = line2LeastSquares;
                line3Height = line3LeastSquares;
            }

	    assert(smallestError != -1);
            if (currentError < smallestError) {
                smallestError = currentError;
                bestJ = j;
                bestK = k;
                line1Height = line1LeastSquares;
                // line2Coeff = line2LeastSquares;
                line3Height = line3LeastSquares;
            }

            line2Values = incrementalValues(j, k, false, line2Values, ys);
            line3Sum = incrementalValues(k, SIZE - 1, true, line3Sum, ys);
        }
    }
    assert(bestJ != -1 && bestK != -1);
    bestJ = bestJ - 1;
    bestK = bestK + 1;
    return std::make_tuple(bestJ, ys[bestJ], xs[bestJ], bestK, ys[bestK], xs[bestK], line1Height, line3Height);
}
double LinearRegression::initialSum(int index1, int index2, const vector<double> &data){
    double sum = 0;

    for (int i = index1; i <= index2; i++) {
        sum += data[i];
    }

    return sum;
}
double* LinearRegression::initialValues(int index1, int index2, const vector<double> &data){
    double* neededValues = new double[4]{0, 0, 0, 0}; // [x, y, xy, xSquared]

    for (int i = index1; i <= index2; i++) {
        neededValues[0] += i;
        neededValues[1] += data[i];
        neededValues[2] += i*data[i];
        neededValues[3] += i*i;
    }

    return neededValues;
}
double LinearRegression::incrementalValues(int oldIndex1, int oldIndex2, bool index1Change, double sum, const vector<double> &data){
    if (index1Change) {
        sum -= data[oldIndex1];
    }

    else {
        sum += data[oldIndex2+1];
    }

    return sum;
}
double* LinearRegression::incrementalValues(int oldIndex1, int oldIndex2, bool index1Change, double values[], const vector<double> &data){
    if (index1Change) {
        values[0] -= oldIndex1;
        values[1] -= data[oldIndex1];
        values[2] -= oldIndex1*data[oldIndex1];
        values[3] -= oldIndex1*oldIndex1;
    }

    else {
        values[0] += oldIndex2+1;
        values[1] += data[oldIndex2+1];
        values[2] += (oldIndex2+1)*data[oldIndex2+1];
        values[3] += (oldIndex2+1)*(oldIndex2+1);
    }

    return values;
}
double LinearRegression::linearLeastSquares(double sum, int n){
    double yIntercept = sum/n;

    return yIntercept;
}
double* LinearRegression::linearLeastSquares(double values[], int n){
    double* averages = new double[4]{values[0]/n, values[1]/n, values[2]/n, values[3]/n}; // [xBar, yBar, xyBar, xSquaredBar]
    double* linearConstants = new double[2]{0, 0}; // [beta, alpha]

    linearConstants[0] = (averages[2] - (averages[0]*averages[1])) / (averages[3] - (averages[0]*averages[0]));
    linearConstants[1] = averages[1] - (linearConstants[0]*averages[0]);

    return linearConstants;
}
double LinearRegression::leastSquaresError(int index1, int index2, double constant, const vector<double> &data){
    double residual;
    double error = 0;

    for (int i = index1; i <= index2; i++) {
        residual = data[i] - constant;

        error += residual*residual;
    }

    return error;
}
double LinearRegression::leastSquaresError(int index1, int index2, double constants[], const vector<double> &data){
    double residual;
    double error = 0;

    for (int i = index1; i <= index2; i++) {
        residual = data[i] - constants[1] - (constants[0]*i);

        error += residual*residual;
    }

    return error;
}
