//
// Created by Hudson Hughes on 8/2/16.
//
#include <tuple>
#include <vector>
#include <cmath>
#include <assert.h>
#include "LinearRegression.hpp"
using namespace std;



LinearRegression::Model::Model(double glMinT, double gmMaxT, double glMinP, double glMaxP, int numSamples):
    goldilocksMinTemp(glMinT), goldilocksMaxTemp(gmMaxT), goldilocksMinPBad(glMinP), goldilocksMaxPBad(glMaxP),
    numSamples(numSamples) {}


double LinearRegression::Model::interpolateWithinGoldilocks(double pBad, bool inLogScale) const {
    if (pBad <= goldilocksMinPBad) return goldilocksMinTemp;
    if (pBad >= goldilocksMaxPBad) return goldilocksMaxTemp;
    double xmin = inLogScale ? log10(goldilocksMinTemp) : goldilocksMinTemp;
    double xmax = inLogScale ? log10(goldilocksMaxTemp) : goldilocksMaxTemp;

    double yfrac = (pBad-goldilocksMinPBad)/(goldilocksMaxPBad-goldilocksMinPBad);
    double xres = xmin+(yfrac*(xmax-xmin));
    return inLogScale ? pow(10, xres) : xres;
}

void LinearRegression::Model::print() const {
    cout << "Goldilocks range: (temp " << goldilocksMinTemp << " pBad " << goldilocksMinPBad << ") to ";
    cout << "(temp " << goldilocksMaxTemp << " pBad " << goldilocksMaxPBad << ")" << endl;
}

LinearRegression::Model LinearRegression::bestFit(const multimap<double, double>& tempToPBad, bool fitTempInLogSpace) {
    vector<double> temps(0);
    vector<double> pBads(0);
    for (auto it : tempToPBad) {
        double temp = it.first;
        temps.push_back(fitTempInLogSpace ? log10(temp) : temp);
        pBads.push_back(it.second);
    }

    int n = temps.size();
    if (n <= 4) throw runtime_error("too few samples for regression");

    double smallestError = -1;
    int bestJ = -1, bestK = -1;
    double bestLine1Height = -1, bestLine3Height = -1;

    double line1PBadSum = pBads[0];
    for (int j = 1; j < n - 2; j++) {
        line1PBadSum += pBads[j];
        double line1Height = line1PBadSum/(j+1);
        double line1Error = flatLineLeastSquaresError(pBads, 0, j, line1Height);

        double line2tempSum = temps[j]+temps[j+1];
        double line2pBadSum = pBads[j+1]+pBads[j+2];
        double line2prodSum = temps[j+1]*pBads[j+1] + temps[j+2]*pBads[j+2];
        double line2sqTempSum = temps[j+1]*temps[j+1] + temps[j+2]*temps[j+2];

        double line3PBadSum = rangeSum(pBads, j+1, n-1);

        for (int k = j+1; k < n - 1; k++) {
            vector<double> line2SlopeIntercept =
                linearLeastSquares(line2tempSum, line2pBadSum, line2prodSum, line2sqTempSum, k-j+1);

            double line3Height = line3PBadSum/(n-k);

            double line2Error = leastSquaresError(temps, pBads, j, k, line2SlopeIntercept);
            double line3Error = flatLineLeastSquaresError(pBads, k, n - 1, line3Height);

            double currentError = line1Error + line2Error + line3Error;

            if (smallestError == -1 or currentError < smallestError) {
                smallestError = currentError;
                bestJ = j;
                bestK = k;
                bestLine1Height = line1Height;
                bestLine3Height = line3Height;
            }

            line2tempSum += temps[k+1];
            line2pBadSum += pBads[k+1];
            line2prodSum += temps[k+1]*pBads[k+1];
            line2sqTempSum += temps[k+1]*temps[k+1];

            line3PBadSum -= pBads[k];
        }
    }
 
    //push transition temps outwards by one sample... not sure why -Nil
    bestJ = bestJ - 1;
    bestK = bestK + 1;

    double glMinT = fitTempInLogSpace ? pow(10, temps[bestJ]) : temps[bestJ];
    double glMaxT = fitTempInLogSpace ? pow(10, temps[bestK]) : temps[bestK];
    return Model(glMinT, glMaxT, bestLine1Height, bestLine3Height, n);
}

double LinearRegression::rangeSum(const vector<double> &v, int index1, int index2){
    double sum = 0;
    for (int i = index1; i <= index2; i++) {
        sum += v[i];
    }
    return sum;
}

vector<double> LinearRegression::linearLeastSquares(double xSum, double ySum, double xySum, double xxSum, int n) {
    double xAvg = xSum/n, yAvg = ySum/n, xyAvg = xySum/n, xxAvg = xxSum/n;

    //I haven't checked this formula -Nil
    double slope = (xyAvg - xAvg*yAvg) / (xxAvg - xAvg*xAvg);
    double intercept = yAvg - slope*xAvg;

    return {slope, intercept};
}

double LinearRegression::flatLineLeastSquaresError(const vector<double> &pBads, int index1, int index2, double lineHeight) {
    double error = 0;
    for (int i = index1; i <= index2; i++) {
        double residual = pBads[i] - lineHeight;
        error += residual*residual;
    }
    return error;
}

double LinearRegression::leastSquaresError(const vector<double> &temps, const vector<double> &pBads, int index1, int index2, vector<double> slopeIntercept) {
    double slope = slopeIntercept[0], intercept = slopeIntercept[1];

    double error = 0;
    for (int i = index1; i <= index2; i++) {
        double residual = pBads[i] - (intercept + slope*temps[i]);
        error += residual*residual;
    }
    return error;
}
