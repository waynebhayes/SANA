//
// Created by Hudson Hughes on 8/2/16.
//
#include <tuple>
#include <vector>
#include <unordered_map> 
#include <array>
#include "LinearRegression.hpp"

using namespace std;

LinearRegression::LinearRegression(const unordered_map<double, double> &graph): chart(graph){ }

tuple<int, double, double, int, double, double, double, double> LinearRegression::Run(){
    vector<double> scores;
    vector<double> temperatures;
    int chartSize = chart.size();

    for (const auto &iterator : chart) {
        temperatures.push_back(iterator.first);
        scores.push_back(iterator.second);
    }

    double line1Sum;
    array<double, 4> line2Values;
    double line3Sum;

    double line1LeastSquares;
    array<double, 2> line2LeastSquares;
    double line3LeastSquares;

    double line1Error;
    double line2Error;
    double line3Error;

    double smallestError;
    int bestJ;
    int bestK;
    double line1Height;
    array<double, 2> line2Coeff;
    double line3Height;
    double currentError;

    for (int j = 1; j < chartSize - 2; j++) {
        line1Sum = InitialSum(0, j, scores);
        line2Values = InitialValues(j, j+1, scores);
        line3Sum = InitialSum(j+1, chartSize - 1, scores);

        line1LeastSquares = line1Sum/j+1;
        line1Error = LeastSquaresError(0, j, line1LeastSquares, scores);

        for (int k = j+1; k < chartSize - 1; k++) {
            line2LeastSquares = LinearLeastSquares(line2Values, k-j+1);
            line3LeastSquares = line3Sum/(chartSize-k);

            line2Error = LeastSquaresError(j, k, line2LeastSquares, scores);
            line3Error = LeastSquaresError(k, chartSize - 1, line3LeastSquares, scores);

            currentError = line1Error + line2Error + line3Error;

            if (j == 1 && k == 2) {
                smallestError = currentError;
                bestJ = j;
                bestK = k;
                line1Height = line1LeastSquares;
                line2Coeff = line2LeastSquares;
                line3Height = line3LeastSquares;
            }

            if (currentError < smallestError) {
                smallestError = currentError;
                bestJ = j;
                bestK = k;
                line1Height = line1LeastSquares;
                line2Coeff = line2LeastSquares;
                line3Height = line3LeastSquares;
            }

            line2Values = IncrementalValues(j, k, false, line2Values, scores);
            line3Sum = IncrementalValues(k, chartSize - 1, true, line3Sum, scores);
        }
    }
    bestJ = bestJ - 1;
    bestK = bestK + 1;
    return make_tuple(bestJ, scores[bestJ], temperatures[bestJ], bestK, scores[bestK], temperatures[bestK], line1Height, line3Height);
}

double LinearRegression::InitialSum(const int &index1, const int &index2, const vector<double> &data){
    double sum = 0;

    for (int i = index1; i <= index2; i++) {
        sum += data[i];
    }

    return sum;
}

array<double, 4> LinearRegression::InitialValues(const int &index1, const int &index2, const vector<double> &data){
    array<double,4> neededValues = {0, 0, 0, 0}; // [x, y, xy, xSquared]

    for (int i = index1; i <= index2; i++) {
        neededValues[0] += i;
        neededValues[1] += data[i];
        neededValues[2] += i*data[i];
        neededValues[3] += i*i;
    }

    return neededValues;
}

double LinearRegression::IncrementalValues(const int &oldIndex1, const int &oldIndex2, const bool &index1Change, const double &sum, const vector<double> &data){
    return sum - data[oldIndex1] ? index1Change : sum - data[oldIndex2+1];
}

array<double, 4> LinearRegression::IncrementalValues(const int &oldIndex1, const int &oldIndex2, const bool &index1Change, const array<double, 4> &values, const vector<double> &data){
    array<double,4> toReturn;
    if (index1Change) {
        toReturn[0] -= oldIndex1;
        toReturn[1] -= data[oldIndex1];
        toReturn[2] -= oldIndex1*data[oldIndex1];
        toReturn[3] -= oldIndex1*oldIndex1;
    }

    else {
        toReturn[0] += oldIndex2+1;
        toReturn[1] += data[oldIndex2+1];
        toReturn[2] += (oldIndex2+1)*data[oldIndex2+1];
        toReturn[3] += (oldIndex2+1)*(oldIndex2+1);
    }

    return toReturn;
}

array<double, 2> LinearRegression::LinearLeastSquares(const array<double, 4> &values, int n){
    array<double, 4> averages = {values[0]/n, values[1]/n, values[2]/n, values[3]/n}; // [xBar, yBar, xyBar, xSquaredBar]
    array<double, 2> linearConstants = {0, 0}; // [beta, alpha]

    linearConstants[0] = (averages[2] - (averages[0]*averages[1])) / (averages[3] - (averages[0]*averages[0]));
    linearConstants[1] = averages[1] - (linearConstants[0]*averages[0]);

    return linearConstants;
}

double LinearRegression::LeastSquaresError(int index1, int index2, double constant, const vector<double> &data){
    double residual;
    double error = 0;

    for (int i = index1; i <= index2; i++) {
        residual = data[i] - constant;

        error += residual*residual;
    }

    return error;
}

double LinearRegression::LeastSquaresError(int index1, int index2, const array<double, 2> constants, const vector<double> &data){
    double residual;
    double error = 0;

    for (int i = index1; i <= index2; i++) {
        residual = data[i] - constants[1] - (constants[0]*i);

        error += residual*residual;
    }

    return error;
}

