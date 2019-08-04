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

    struct Model {
        Model(double glMinT, double gmMaxT, double glMinP, double glMaxP, int numSamples);
        const double goldilocksMinTemp;
        const double goldilocksMaxTemp;
        const double goldilocksMinPBad;
        const double goldilocksMaxPBad;
        const int numSamples;

        //assuming pBad is between goldilocks minPBad and maxPBad
        //interpolates the temperature that gives rise to that pBad
        double interpolateWithinGoldilocks(double pBad, bool inLogScale = true) const;
        void print() const;
    };

    static Model bestFit(const multimap<double, double>& tempToPBad, bool fitTempInLogSpace = true);

private:

    static double rangeSum(const vector<double> &v, int index1, int index2);
    static vector<double> linearLeastSquares(double xSum, double ySum, double xySum, double xxSum, int n);
    static double flatLineLeastSquaresError(const vector<double> &pBads, int index1, int index2, double lineHeight);
    static double leastSquaresError(const vector<double> &temps, const vector<double> &pBads, int index1, int index2, vector<double> slopeIntercept);

};


#endif