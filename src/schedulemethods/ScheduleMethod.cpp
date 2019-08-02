#include <iostream>
#include <assert.h> 

#include "ScheduleMethod.hpp"
#include "../utils/Timer.hpp"
#include "../utils/LinearRegression.hpp"

using namespace std;

multimap<double, double> ScheduleMethod::allTempToPBad = multimap<double, double> (); 

ScheduleMethod::ScheduleMethod(SANA *const sana): sana(sana),
    targetInitialPBad(DEFAULT_TARGET_INITIAL_PBAD), targetFinalPBad(DEFAULT_TARGET_FINAL_PBAD),
    TInitialTime(0), TFinalTime(0), TInitialSamples(0), TFinalSamples(0),
    tempToPBad(),
    hasComputedTInitial(false), hasComputedTFinal(false) {}

void ScheduleMethod::setTargetInitialPBad(double pBad) {
    targetInitialPBad = pBad;
    hasComputedTInitial = false;
}

void ScheduleMethod::setTargetFinalPBad(double pBad) {
    targetFinalPBad = pBad;
    hasComputedTFinal = false;
}

double ScheduleMethod::getTInitial() {
    if (not hasComputedTInitial) {
        int currSamples = tempToPBad.size();
        Timer T;
        T.start();
        
        computeTInitial();
        
        TInitialTime = T.elapsed();
        TInitialSamples = tempToPBad.size() - currSamples;
        hasComputedTInitial = true;
    }
    return TInitial;
}

double ScheduleMethod::getTFinal() {
    if (not hasComputedTFinal) {
        int currSamples = tempToPBad.size();
        Timer T;
        T.start();

        computeTFinal();

        TFinalTime = T.elapsed();
        TFinalSamples = tempToPBad.size() - currSamples;
        hasComputedTFinal = true;
    }
    return TFinal;
}

double ScheduleMethod::computeTempForPBad(double pBad) {
    throw runtime_error("functionality not implemented for this method");
}

double ScheduleMethod::getPBad(double temp, double maxTime, int logLevel) {
    double res = sana->getPBad(temp, maxTime, logLevel);
    tempToPBad.insert({temp, res});
    allTempToPBad.insert({temp, res});
    return res;
}

/* Returns a temperature of the form 'base'^k (for some integer k) that gives rise to
a pBad that bounds 'targetPBad' above or below. More precisely, 
- if 'nextAbove' is true, it returns the smallest such temp that gives rise to a pBad above 'targetPBad'
- if 'nextAbove' is false, it retuns the largest such temp that gives rise to a pBad below 'targetPBad'
higher 'base' -> quicker search, but more coarse bound; defaults to 10.
*/
double ScheduleMethod::doublingMethod(double targetPBad, bool nextAbove, double base, double getPBadTime) {
    
    //use as starting value the temp in the tempToPBad map that has a closest pBad to the target pBad
    //if the map is empty, just start with 1
    double startTemp = 1;
    double smallestPBadDiff = 2;
    double startPBad;
    bool initStartPBad = false;
    for (auto tempPBadPair = tempToPBad.begin(); tempPBadPair != tempToPBad.end(); tempPBadPair++) {
        double temp = tempPBadPair->first;
        double pBad = tempPBadPair->second;
        double pBadDiff = abs(pBad-targetPBad);
        if (pBadDiff < smallestPBadDiff) {
            startTemp = temp;
            smallestPBadDiff = pBadDiff;
            startPBad = pBad;
            initStartPBad = true;
        }
    }

    //just in case, as these would give math errors
    if (base == 0) base = 10;
    if (startTemp == 0) startTemp = 1;

    //turn 'startTemp' into the closest power of 'base' below 'startTemp'
    double startTempLog = log(startTemp)/log(base); //log_b a = log a / log b
    startTempLog = floor(startTempLog);
    startTemp = pow(base, startTempLog);

    double temp = startTemp;
    double priorTemp = temp;
    double pBad;
    if (initStartPBad) pBad = startPBad;
    else pBad = getPBad(temp, getPBadTime);

    if (pBad < targetPBad) {
        while (pBad < targetPBad) {
            priorTemp = temp;
            temp *= base;
            pBad = getPBad(temp, getPBadTime);
        }
        if (nextAbove) return temp;
        return priorTemp;      
    } else {
        while (pBad > targetPBad) {
            priorTemp = temp;
            temp /= base;
            pBad = getPBad(temp, getPBadTime);
        }
        if (nextAbove) return priorTemp;
        return temp;
    }
}

double ScheduleMethod::pBadBinarySearch(double targetPBad) {
    cerr<<"Using pBad binary search for target pBad = "<<targetPBad<< endl;

    //customizable parameters
    bool logScale = true;
    double convergenceTime = 2;
    double tolerance = 0.01;

    double targetRangeTop = targetPBad*(1+tolerance);
    double targetRangeBottom = targetPBad*(1-tolerance);

    //establish starting range
    double highTemp = doublingMethod(targetPBad, true, 100, convergenceTime);
    double lowTemp = doublingMethod(targetPBad, false, 100, convergenceTime);
    double highPBad = (tempToPBad.find(highTemp))->second;
    double lowPBad = (tempToPBad.find(lowTemp))->second;

    //make sure that starting bounds enclose the target range
    bool areGoodBounds = lowTemp < highTemp and lowPBad < targetRangeBottom and highPBad > targetRangeTop;
    while (not areGoodBounds) {
        highTemp *= 10;
        lowTemp /= 10;
        highPBad = getPBad(highTemp, convergenceTime);
        lowPBad = getPBad(lowTemp, convergenceTime);
        areGoodBounds = lowTemp < highTemp and lowPBad < targetRangeBottom and highPBad > targetRangeTop;
    }

    double midTemp = logScale ? exp((log(highTemp)+log(lowTemp))/2.0) : (highTemp+lowTemp)/2.0;
    double midPBad = getPBad(midTemp, convergenceTime);

    if (highPBad >= targetRangeBottom and highPBad <= targetRangeTop) return highTemp;
    if (lowPBad >= targetRangeBottom and lowPBad <= targetRangeTop) return lowTemp;
    if (midPBad >= targetRangeBottom and midPBad <= targetRangeTop) return midTemp;

    cerr<<"Target range: ("<<targetRangeBottom<<", "<<targetRangeTop<<")"<<endl;
    cerr<<"Start search bounds: temps: ("<<lowTemp<<", "<<midTemp<<", "<<highTemp;
    cerr<<") pbads: ("<<lowPBad<<","<<midPBad<<","<<highPBad<<")"<<endl;

    //in the search, the following invariants hold:
    //(1) lowTemp < midTemp < highTemp ; (2) lowPBad < targetRange < highPBad ;
    //(3) lowPBad < midPBad < highPBad ; (4) midPBad not in targetRange
    // at this moment, (1), (2) and (4) hold. (3) we have not checked
    //(1) will be maintained by definition of binary search and (4) will be easily checked
    //(2) will hold as long as (3) is maintained
    //(3) can go wrong due to noise or if the temperatures are very close.
    //If it does, we recompute all 3 pbads and hope it's fixed.
    //We try this X times, if we can't get invariant (3) after X attempts, we call it a day

    bool invariants = lowPBad <= midPBad and midPBad <= highPBad and
                        lowPBad < targetRangeBottom and highPBad > targetRangeTop;
    int failCount = 0;
    while (not invariants) {
        failCount++;
        if (failCount == 3) {
            cerr << "Binary search for pBad could not establish a reasonable starting range --" << endl;
            cerr << "Output temperature is likely not good" << endl;
            return highTemp;
        }
        highPBad = getPBad(highTemp, convergenceTime);
        if (highPBad >= targetRangeBottom and highPBad <= targetRangeTop) return highTemp;
        lowPBad = getPBad(lowTemp, convergenceTime);
        if (lowPBad >= targetRangeBottom and lowPBad <= targetRangeTop) return lowTemp;
        midPBad = getPBad(midTemp, convergenceTime);
        if (midPBad >= targetRangeBottom and midPBad <= targetRangeTop) return midTemp;

        invariants = lowPBad <= midPBad and midPBad <= highPBad and
                        lowPBad < targetRangeBottom and highPBad > targetRangeTop;
    }

    //now all invariants hold. Ready to start search
    int maxGetPBads = 10;
    int numGetPBads = 0;
    while (numGetPBads < maxGetPBads) {
        if (midPBad < targetRangeBottom) {
            lowTemp = midTemp;
            lowPBad = midPBad;
        } else if (midPBad > targetRangeTop) {
            highTemp = midTemp;
            highPBad = midPBad;
        } else throw runtime_error("invariant not maintained in pbad binary search");

        midTemp = logScale ? exp((log(highTemp)+log(lowTemp))/2.0) : (highTemp+lowTemp)/2.0;
        midPBad = getPBad(midTemp, convergenceTime);
        numGetPBads++;
        if (midPBad >= targetRangeBottom and midPBad <= targetRangeTop) return midTemp;

        invariants = lowPBad <= midPBad and midPBad <= highPBad;
        failCount = 0;
        while (not invariants) {
            cerr<<"Invalid search range: temps: ("<<lowTemp<<", "<<midTemp<<", "<<highTemp;
            cerr<<") pbads: ("<<lowPBad<<","<<midPBad<<","<<highPBad<<")"<<endl;
            failCount++;
            if (failCount == 2) {
                return midTemp;
            }
            midPBad = getPBad(midTemp, 1.5*convergenceTime);
            numGetPBads++;
            if (midPBad >= targetRangeBottom and midPBad <= targetRangeTop) return midTemp;
            if (midPBad < lowPBad) {
                lowPBad = getPBad(lowTemp, 1.5*convergenceTime);
                numGetPBads++;
                if (lowPBad >= targetRangeBottom and lowPBad <= targetRangeTop) return lowTemp;
            }
            if (midPBad > highPBad) {
                highPBad = getPBad(highTemp, 1.5*convergenceTime);
                numGetPBads++;
                if (highPBad >= targetRangeBottom and highPBad <= targetRangeTop) return highTemp;
            }
            invariants = lowPBad <= midPBad and midPBad <= highPBad and
                         lowPBad < targetRangeBottom and highPBad > targetRangeTop;
            if (invariants) {
                cerr<<"Corrected pbads: ("<<lowPBad<<","<<midPBad<<","<<highPBad<<")"<<endl;
            }
        }
    }
    return highTemp; //upper bound on actual temp
}


void ScheduleMethod::setTInitialTFinalFromRegression(bool useDataFromAllMethods) {

    LinearRegression LR(useDataFromAllMethods ? allTempToPBad : tempToPBad, true);
    tuple<int, double, double, int, double, double, double, double> regressionResult = LR.run();
    //bestJ,scores[bestJ],temps[bestJ],bestK,scores[bestK],temps[bestK],line1Height,line3Height;

    double rangeBottom = pow(10, get<2>(regressionResult));
    double rangeTop = pow(10, get<5>(regressionResult));
    cout << "Goldilocks range: temp (" << rangeBottom << ", " << rangeTop << ")";
    cout << "pBad (" << get<6>(regressionResult) << ", " << get<7>(regressionResult) << ")" << endl;
    
    if (getPBad(rangeTop, 2) >= targetInitialPBad) TInitial = rangeTop;
    else {
        //set as TInitial the lowest temp in tempToPBad which is in the random region AND
        //has a pBad bigger (or eq) than the target initial pBad
        bool found = false;
        for (auto pair : tempToPBad) {
            //multimap is an ordered structure, so it iterates keys from low to high
            if (pair.first >= rangeTop and pair.second >= targetInitialPBad) {
                TInitial = pair.first;
                found = true;
                break;
            }
        }
        if (not found) {
            //If no temp is found with both conditions, use
            //the lowest temp with a pBad bigger than the target initial pBad
            for (auto pair : tempToPBad) {
                if (pair.second >= targetInitialPBad) {
                    TInitial = pair.first;
                    found = true;
                }
            }        
        }
        if (not found) {
            //whacky case where no temp in tempToPBad has a pBad above the target.
            //just take the largest... 
            TInitial = pow(10, tempToPBad.rbegin()->first);
        }
    }

    TFinal = rangeBottom;
    double distFromTarget = abs(targetFinalPBad - getPBad(rangeBottom, 2));
    for (auto pair : tempToPBad) {
        if (abs(targetFinalPBad - pair.second) < distFromTarget and
                pair.first <= TInitial) {
            TFinal = pair.first;
            distFromTarget = abs(targetFinalPBad - pair.second);
        }
    }
}

void ScheduleMethod::populatePBadCurve() {
    const double HIGH_PBAD_LIMIT = 0.99999;
    const double LOW_PBAD_LIMIT = 1e-10;
    cout << "Populating PBad curve" << endl;
    double highTemp = doublingMethod(HIGH_PBAD_LIMIT, false);
    double lowTemp = doublingMethod(LOW_PBAD_LIMIT, true);
    double numSteps = pow(10, abs(log10(lowTemp)) + abs(log10(highTemp)));
    for (int T_i = 0; T_i <= log10(numSteps); T_i++) {
        double logTemp = log10(lowTemp) + T_i*(log10(highTemp)-log10(lowTemp))/log10(numSteps);
        double temp = pow(10, logTemp);
        getPBad(temp, 2.0);
    }
}



void ScheduleMethod::printScheduleStatistics() {
    cout << "TInitial found in " << TInitialTime << "s for target pBad " << targetInitialPBad << ": " << endl;
    getPBad(TInitial, 2);
    cout << "TFinal found in " << TFinalTime << "s for target pBad " << targetFinalPBad << ": " << endl;
    getPBad(TFinal, 2);
    cout << "TDecay needed to traverse this range: " << -log(TFinal/TInitial) << endl;
    cout << endl;
}

vector<double> ScheduleMethod::dataForComparison() {

    double TIniPBad = getPBad(TInitial, 5);
    double TIniPBadRelative = TIniPBad/targetInitialPBad;
    double TFinPBad = getPBad(TFinal, 5);
    double TFinPBadRelative = TFinPBad/targetFinalPBad;
    double totalTime = TInitialTime + TFinalTime;
    int totalSamples = TInitialSamples + TFinalSamples;

    return 
        { TInitial, TIniPBad, TIniPBadRelative, (double)TInitialSamples, TInitialTime,
          TFinal,   TFinPBad, TFinPBadRelative, (double)TFinalSamples, TFinalTime,
          (double)totalSamples, totalTime };    
}
