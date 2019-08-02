#include <iostream>
#include <map>

#include "LinearRegressionVintage.hpp"
#include "../utils/LinearRegression.hpp"

using namespace std;

LinearRegressionVintage::LinearRegressionVintage(SANA *const sana):
    ScheduleMethod(sana) {}

void LinearRegressionVintage::computeTInitial() {
    computeBoth();
    hasComputedTFinal = true;
}

void LinearRegressionVintage::computeTFinal() {
    computeBoth();
    hasComputedTInitial = true;
}

void LinearRegressionVintage::computeBoth() {
    const double HIGH_PBAD_LIMIT = 0.99999;
    const double LOW_PBAD_LIMIT = 1e-10;

    //if(score == "pareto") //Running in pareto mode makes this function really slow
    //  return;             //and I don't know why, but sometimes I disable using this.
    //                      //otherwise my computer is very slow.

    cout << "Finding Upper Bound of the Temperature ... " << endl;
    double log10HighTemp = log10(doublingMethod(HIGH_PBAD_LIMIT, false));
    cout << "Finding Lower Bound of the Temperature ... " << endl;
    double log10LowTemp = log10(doublingMethod(LOW_PBAD_LIMIT, true));

    double log10NumSteps = abs(log10LowTemp) + abs(log10HighTemp);

    cout << "HIGH TEMP = " << pow(10, log10HighTemp) << " LOW TEMP = " << pow(10, log10LowTemp) << endl;
    cout << "NUM OF STEPS = " << pow(10, log10NumSteps) << endl;
    cout << "Sampling " << (int) (1+log10NumSteps) << " pBads from " << pow(10, log10LowTemp);
    cout << " to " << pow(10, log10HighTemp) <<" for linear regression" << endl;

    int T_i;
    double log_temp;
    map<double, double> pbadMap;

    for(T_i = 0; T_i <= log10NumSteps; T_i++){
    log_temp = log10LowTemp + T_i*(log10HighTemp-log10LowTemp)/log10NumSteps;
        pbadMap[log_temp] = getPBad(pow(10, log_temp), 2.0);
        // cout << T_i << " temperature: " << pow(10, log_temp) << " pBad: " << pbadMap[log_temp] << " score: " << eval(*A) << endl;
    }
    for (T_i=0; T_i <= log10NumSteps; T_i++){
    log_temp = log10LowTemp + T_i*(log10HighTemp-log10LowTemp)/log10NumSteps;
        if(pbadMap[log_temp] > targetFinalPBad)
            break;
    }

    double binarySearchLeftEnd = log10LowTemp + (T_i-1)*(log10HighTemp-log10LowTemp)/log10NumSteps;
    double binarySearchRightEnd = log_temp;
    double mid = (binarySearchRightEnd + binarySearchLeftEnd) / 2;
    cout << "Increasing sample density near TFinal. " << " range: (" << pow(10, binarySearchLeftEnd) << ", " << pow(10, binarySearchRightEnd) << ")" << endl;
    for(int j = 0; j < 4; ++j) {
        double temperature = pow(10, mid);
        double probability = getPBad(temperature, 2.0);
        pbadMap[mid] = probability;
        if(probability > targetFinalPBad) binarySearchRightEnd = mid;
        else binarySearchLeftEnd = mid;
        mid = (binarySearchRightEnd + binarySearchLeftEnd) / 2;
    }
    for (T_i = log10NumSteps; T_i >= 0; T_i--){
        log_temp = log10LowTemp + T_i*(log10HighTemp-log10LowTemp)/log10NumSteps;
        if(pbadMap[log_temp] < targetInitialPBad)
            break;
    }

    binarySearchLeftEnd = log_temp;
    binarySearchRightEnd = log10LowTemp + (T_i+1)*(log10HighTemp-log10LowTemp)/log10NumSteps;
    mid = (binarySearchRightEnd + binarySearchLeftEnd) / 2;
    cout << "Increasing sample density near TInitial. " << "range: (" << pow(10, binarySearchLeftEnd) << ", " << pow(10, binarySearchRightEnd) << ")" << endl;
    for(int j = 0; j < 4; ++j){
        double temperature = pow(10, mid);
        double probability = getPBad(temperature, 2.0);
        pbadMap[mid] = probability;
        if(probability < targetInitialPBad) binarySearchLeftEnd = mid;
        else binarySearchRightEnd = mid;
        mid = (binarySearchRightEnd + binarySearchLeftEnd) / 2;
    }
    LinearRegression linearRegression(pbadMap);
    tuple<int, double, double, int, double, double, double, double> regressionResult = linearRegression.run();
    // bestJ, scores[bestJ], temperatures[bestJ], bestK, scores[bestK], temperatures[bestK], line1Height, line3Height;
    double lowerEnd = get<2>(regressionResult);
    double upperEnd = get<5>(regressionResult);
    cout << "The three lines are: "<<endl;
    cout << "HillClimbing: y= " << get<6>(regressionResult) << " until x= " << pow(10,lowerEnd) << endl;
    cout << "GoldilocksZone: starts above, ends at x= " << pow(10,upperEnd) << endl;
    cout << "RandomRegion: y= " << get<7>(regressionResult) << endl;
    cout << "Left endpoint of linear regression " << pow(10, lowerEnd) << endl;
    cout << "Right endpoint of linear regression " << pow(10, upperEnd) << endl;
    TInitial = pow(10, log10HighTemp);
    for (auto const& keyValue : pbadMap)
    {
        if(keyValue.second >= targetInitialPBad && keyValue.first >= upperEnd){
            TInitial = pow(10, keyValue.first);
            break;
        }
    }
    TFinal = pow(10,log10LowTemp);
    double distanceFromTarget = std::numeric_limits<double>::max();
    for (auto const& keyValue : pbadMap)
    {
        if (distanceFromTarget > abs(targetFinalPBad - keyValue.second) and
                pow(10, keyValue.first) <= TInitial) {
            distanceFromTarget = abs(targetFinalPBad - keyValue.second);
            TFinal = pow(10, keyValue.first);
        }
    }
}