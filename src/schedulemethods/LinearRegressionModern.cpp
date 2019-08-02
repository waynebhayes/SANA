#include <iostream>

#include "LinearRegressionModern.hpp"
#include "../utils/LinearRegression.hpp"

using namespace std;

LinearRegressionModern::LinearRegressionModern(SANA *const sana):
    ScheduleMethod(sana) {}

void LinearRegressionModern::computeTInitial() {
    computeBoth();
    hasComputedTFinal = true;
}

void LinearRegressionModern::computeTFinal() {
    computeBoth();
    hasComputedTInitial = true;
}

void LinearRegressionModern::computeBoth() {
    cout << "Setting TInitial and TFinal by Linear Regression" << endl;
    populatePBadCurve();
    pBadBinarySearch(targetInitialPBad);
    pBadBinarySearch(targetFinalPBad);        
    setTInitialTFinalFromRegression();
}