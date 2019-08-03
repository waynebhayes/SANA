#include <iostream>

#include "LinearRegressionModern.hpp"
#include "../utils/LinearRegression.hpp"

using namespace std;

LinearRegressionModern::LinearRegressionModern(SANA *const sana):
    ScheduleMethod(sana), alreadyComputed(false) {}

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

    populatePBadCurve();
    double remainingTime = maxTime - T.elapsed();
    int remainingSamples = tempToPBad.size()-startSamples;
    pBadBinarySearch(targetInitialPBad, remainingTime/2, remainingSamples/2);
    pBadBinarySearch(targetFinalPBad, remainingTime/2, remainingSamples/2);        
    setTInitialTFinalFromRegression();
}