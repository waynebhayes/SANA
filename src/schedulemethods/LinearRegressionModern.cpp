#include <iostream>

#include "LinearRegressionModern.hpp"
#include "../utils/LinearRegression.hpp"

using namespace std;

LinearRegressionModern::LinearRegressionModern():
    ScheduleMethod(), alreadyComputed(false) {}

void LinearRegressionModern::setTargetInitialPBad(double pBad) {
    ScheduleMethod::setTargetInitialPBad(pBad);
    alreadyComputed = false;
}
void LinearRegressionModern::setTargetFinalPBad(double pBad) {
    ScheduleMethod::setTargetFinalPBad(pBad);
    alreadyComputed = false;
}

void LinearRegressionModern::vComputeTInitial(double maxTime, int maxSamples) {
    if (alreadyComputed) return;
    computeBoth(maxTime, maxSamples); 
    alreadyComputed = true;  
}

void LinearRegressionModern::vComputeTFinal(double maxTime, int maxSamples) {
    if (alreadyComputed) return;
    computeBoth(maxTime, maxSamples); 
    alreadyComputed = true;  
}

void LinearRegressionModern::computeBoth(double maxTime, int maxSamples) {
    Timer T; 
    T.start();
    int startSamples = tempToPBad.size();

    cout << "Populating PBad curve" << endl;
    populatePBadCurve();

    double remainingTime = maxTime - T.elapsed();
    int remainingSamples = maxSamples-(tempToPBad.size()-startSamples);

    cout << "Increasing density near TInitial" << endl;
    pBadBinarySearch(targetInitialPBad, remainingTime/2, remainingSamples/2);
    cout << "Increasing density near TFinal" << endl;
    pBadBinarySearch(targetFinalPBad, remainingTime/2, remainingSamples/2);  

    auto model = LinearRegression::bestFit(tempToPBad);
    cout << "*** Linear Regression Model: ";
    model.print();

    TInitial = tempWithBestLRFit(targetInitialPBad);
    TFinal = tempWithBestLRFit(targetFinalPBad);
}