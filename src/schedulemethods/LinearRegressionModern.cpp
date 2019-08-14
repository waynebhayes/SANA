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

void LinearRegressionModern::vComputeTInitial(ScheduleMethod::Resources maxRes) {
    if (alreadyComputed) return;
    computeBoth(maxRes); 
    alreadyComputed = true;  
}

void LinearRegressionModern::vComputeTFinal(ScheduleMethod::Resources maxRes) {
    if (alreadyComputed) return;
    computeBoth(maxRes); 
    alreadyComputed = true;  
}

void LinearRegressionModern::computeBoth(ScheduleMethod::Resources maxRes) {
    Timer T; 
    T.start();
    int startSamples = tempToPBad.size();

    cout << "Populating PBad curve" << endl;
    populatePBadCurve();

    int remainingSamples = maxRes.numSamples-(tempToPBad.size()-startSamples);
    ScheduleMethod::Resources remRes(remainingSamples, maxRes.runtime - T.elapsed());
    ScheduleMethod::Resources halfRemRes(remRes.numSamples/2, remRes.runtime/2);

    cout << "Increasing density near TInitial" << endl;
    pBadBinarySearch(targetInitialPBad, halfRemRes);

    remRes.numSamples = maxRes.numSamples-(tempToPBad.size()-startSamples);
    remRes.runtime = maxRes.runtime - T.elapsed();
    cout << "Increasing density near TFinal" << endl;
    pBadBinarySearch(targetFinalPBad, remRes);  

    auto model = LinearRegression::bestFit(tempToPBad);
    cout << "*** Linear Regression Model: " << endl;
    model.print();

    TInitial = tempWithBestLRFit(targetInitialPBad);
    TFinal = tempWithBestLRFit(targetFinalPBad);
}