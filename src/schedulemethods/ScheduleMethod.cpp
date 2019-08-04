#include <iostream>
#include <assert.h> 

#include "ScheduleMethod.hpp"
#include "../utils/Timer.hpp"
#include "../utils/LinearRegression.hpp"

using namespace std;

//initialization of static members
multimap<double, double> ScheduleMethod::allTempToPBad = multimap<double, double> (); 
SANA* ScheduleMethod::sana = nullptr;


ScheduleMethod::ScheduleMethod():
    targetInitialPBad(DEFAULT_TARGET_INITIAL_PBAD), targetFinalPBad(DEFAULT_TARGET_FINAL_PBAD),
    errorTol(DEFAULT_ERROR_TOL), sampleTime(DEFAULT_SAMPLE_TIME),
    tempToPBad(),
    TInitialTime(0), TFinalTime(0),
    TInitialSamples(0), TFinalSamples(0) {

}

double ScheduleMethod::computeTInitial(double maxTime, int maxSamples) {
    cout << "Computing TInitial via method " << getName() << endl;
    int currSamples = tempToPBad.size();
    Timer T;
    T.start();
    
    vComputeTInitial(maxTime, maxSamples);
    
    TInitialTime = T.elapsed();
    TInitialSamples = tempToPBad.size() - currSamples;

    cout << "Computed TInitial " << TInitial << " in " << TInitialTime << "s" << endl;
    return TInitial;
}

void ScheduleMethod::vComputeTInitial(double maxTime, int maxSamples) {
    TInitial = computeTempForPBad(targetInitialPBad, maxTime, maxSamples);
}


double ScheduleMethod::computeTFinal(double maxTime, int maxSamples) {
    cout << "Computing TFinal via method " << getName() << endl;
    int currSamples = tempToPBad.size();
    Timer T;
    T.start();

    vComputeTFinal(maxTime, maxSamples);

    TFinalTime = T.elapsed();
    TFinalSamples = tempToPBad.size() - currSamples;

    cout << "Computed TFinal " << TFinal << " in " << TFinalTime << "s" << endl;    
    return TFinal;
}

void ScheduleMethod::vComputeTFinal(double maxTime, int maxSamples) {
    TFinal = computeTempForPBad(targetFinalPBad, maxTime, maxSamples);
}

double ScheduleMethod::computeTempForPBad(double targetPBad, double maxTime, int maxSamples) {
    throw runtime_error("functionality not implemented for this method");
}

double ScheduleMethod::sGetPBad(double temp, double sampleTime) {
    double res = sana->getPBad(temp, sampleTime);
    allTempToPBad.insert({temp, res});
    return res;
}

double ScheduleMethod::getPBad(double temp) {
    double res = sGetPBad(temp, sampleTime);
    tempToPBad.insert({temp, res});
    return res;
}

double ScheduleMethod::targetRangeMin(double targetPBad, double errorTol) {
    return targetPBad*(1-errorTol);
}
double ScheduleMethod::targetRangeMax(double targetPBad, double errorTol) {
    return targetPBad*(1+errorTol);
}
double ScheduleMethod::distToTargetRange(double pBad, double targetPBad, double errorTol) {
    if (isAboveTargetRange(pBad, targetPBad, errorTol))
        return pBad-targetRangeMax(targetPBad, errorTol);
    if (isBelowTargetRange(pBad, targetPBad, errorTol))
        return targetRangeMin(targetPBad, errorTol)-pBad;
    return 0;
}
bool ScheduleMethod::isBelowTargetRange(double pBad, double targetPBad, double errorTol) {
    return pBad < targetRangeMin(targetPBad, errorTol);
}
bool ScheduleMethod::isAboveTargetRange(double pBad, double targetPBad, double errorTol) {
    return pBad > targetRangeMax(targetPBad, errorTol);
}
bool ScheduleMethod::isWithinTargetRange(double pBad, double targetPBad, double errorTol) {
    return !isBelowTargetRange(pBad, targetPBad, errorTol) &&
           !isAboveTargetRange(pBad, targetPBad, errorTol);
}
void ScheduleMethod::printTargetRange(double targetPBad, double errorTol) {
    cout << "(" << targetPBad*(1-errorTol) << ", " << targetPBad*(1+errorTol) << ")";
}





/* Returns a temperature of the form 'base'^k (for some integer k) that gives rise to
a pBad that bounds 'targetPBad' above or below. More precisely, 
- if 'nextAbove' is true, it returns the smallest such temp that gives rise to a pBad above 'targetPBad'
- if 'nextAbove' is false, it retuns the largest such temp that gives rise to a pBad below 'targetPBad'
higher 'base' -> quicker search, but more coarse bound; defaults to 10.
*/
double ScheduleMethod::doublingMethod(double targetPBad, bool nextAbove, double base) {
    
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
    else pBad = getPBad(temp);

    if (pBad < targetPBad) {
        while (pBad < targetPBad) {
            priorTemp = temp;
            temp *= base;
            pBad = getPBad(temp);
        }
        if (nextAbove) return temp;
        return priorTemp;      
    } else {
        while (pBad > targetPBad) {
            priorTemp = temp;
            temp /= base;
            pBad = getPBad(temp);
        }
        if (nextAbove) return priorTemp;
        return temp;
    }
}

double ScheduleMethod::pBadBinarySearch(double targetPBad, double maxTime, int maxSamples) {
    Timer T;
    T.start();
    int startSamples = tempToPBad.size();

    //customizable parameters
    bool logScale = true;
    // double tolerance = 0.01;

    //establish starting range
    double highTemp = doublingMethod(targetPBad, true, 100);
    double lowTemp = doublingMethod(targetPBad, false, 100);
    double highPBad = (tempToPBad.find(highTemp))->second;
    double lowPBad = (tempToPBad.find(lowTemp))->second;

    //make sure that starting bounds enclose the target range
    bool areGoodBounds = lowTemp < highTemp and isBelowTargetRange(lowPBad, targetPBad, errorTol)
                                            and isAboveTargetRange(highPBad, targetPBad, errorTol);

    while (not areGoodBounds) {
        highTemp *= 10;
        lowTemp /= 10;
        highPBad = getPBad(highTemp);
        lowPBad = getPBad(lowTemp);
        areGoodBounds = lowTemp < highTemp and isBelowTargetRange(lowPBad, targetPBad, errorTol)
                                           and isAboveTargetRange(highPBad, targetPBad, errorTol);
    }

    double midTemp = logScale ? exp((log(highTemp)+log(lowTemp))/2.0) : (highTemp+lowTemp)/2.0;
    double midPBad = getPBad(midTemp);

    if (isWithinTargetRange(highPBad, targetPBad, errorTol)) return highTemp;
    if (isWithinTargetRange(lowPBad, targetPBad, errorTol)) return lowTemp;
    if (isWithinTargetRange(midPBad, targetPBad, errorTol)) return midTemp;

    cerr<<"Target range: ("<<targetRangeMin(targetPBad, errorTol)<<", ";
    cerr<<targetRangeMax(targetPBad, errorTol)<<")"<<endl;
    cerr<<"Start search bounds: temps: ("<<lowTemp<<", "<<midTemp<<", "<<highTemp<<") ";
    cerr<<"pbads: ("<<lowPBad<<", "<<midPBad<<", "<<highPBad<<")"<<endl;

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
                        isBelowTargetRange(lowPBad, targetPBad, errorTol) and
                        isAboveTargetRange(highPBad, targetPBad, errorTol);
    int failCount = 0;
    while (not invariants) {
        failCount++;
        if (failCount == 3) {
            cerr << "Binary search for pBad could not establish a reasonable starting range --" << endl;
            cerr << "Output temperature is likely not good" << endl;
            double dlow = abs(lowPBad - targetPBad);
            double dmid = abs(midPBad - targetPBad);
            double dhigh = abs(highPBad - targetPBad);
            if (dlow < max(dmid,dhigh)) return lowTemp;
            else if (dmid < dhigh) return midTemp;
            return highTemp;
        }
        highPBad = getPBad(highTemp);
        if (isWithinTargetRange(highPBad, targetPBad, errorTol)) return highTemp;
        lowPBad = getPBad(lowTemp);
        if (isWithinTargetRange(lowPBad, targetPBad, errorTol)) return lowTemp;
        midPBad = getPBad(midTemp);
        if (isWithinTargetRange(midPBad, targetPBad, errorTol)) return midTemp;

        invariants = lowPBad <= midPBad and midPBad <= highPBad and
                        isBelowTargetRange(lowPBad, targetPBad, errorTol) and
                        isAboveTargetRange(highPBad, targetPBad, errorTol);
    }


    //now all invariants hold. Ready to start search
    while (T.elapsed() < maxTime and (int)tempToPBad.size() - startSamples < maxSamples) {
        if (isBelowTargetRange(midPBad, targetPBad, errorTol)) {
            lowTemp = midTemp;
            lowPBad = midPBad;
        } else if (isAboveTargetRange(midPBad, targetPBad, errorTol)) {
            highTemp = midTemp;
            highPBad = midPBad;
        } else throw runtime_error("invariant not maintained in pbad binary search");

        midTemp = logScale ? exp((log(highTemp)+log(lowTemp))/2.0) : (highTemp+lowTemp)/2.0;
        midPBad = getPBad(midTemp);
        if (isWithinTargetRange(midPBad, targetPBad, errorTol)) return midTemp;

        invariants = lowPBad <= midPBad and midPBad <= highPBad;
        failCount = 0;
        while (not invariants) {
            cerr<<"Invalid search range: temps: ("<<lowTemp<<", "<<midTemp<<", "<<highTemp;
            cerr<<") pbads: ("<<lowPBad<<", "<<midPBad<<", "<<highPBad<<")"<<endl;
            failCount++;
            if (failCount == 3) {
                double dlow = abs(lowPBad - targetPBad);
                double dmid = abs(midPBad - targetPBad);
                double dhigh = abs(highPBad - targetPBad);
                if (dlow < max(dmid,dhigh)) return lowTemp;
                else if (dmid < dhigh) return midTemp;
                return highTemp;
            }
            midPBad = getPBad(midTemp);
            if (isWithinTargetRange(midPBad, targetPBad, errorTol)) return midTemp;
            if (midPBad < lowPBad) {
                lowPBad = getPBad(lowTemp);
                if (isWithinTargetRange(lowPBad, targetPBad, errorTol)) return lowTemp;
            }
            if (midPBad > highPBad) {
                highPBad = getPBad(highTemp);
                if (isWithinTargetRange(highPBad, targetPBad, errorTol)) return highTemp;
            }
            invariants = lowPBad <= midPBad and midPBad <= highPBad and
                         isBelowTargetRange(lowPBad, targetPBad, errorTol) and
                         isAboveTargetRange(highPBad, targetPBad, errorTol);
            if (invariants) {
                cerr<<"Corrected pbads: ("<<lowPBad<<", "<<midPBad<<", "<<highPBad<<")"<<endl;
            }
        }
    }
    double dlow = abs(lowPBad - targetPBad);
    double dmid = abs(midPBad - targetPBad);
    double dhigh = abs(highPBad - targetPBad);
    if (dlow < max(dmid,dhigh)) return lowTemp;
    else if (dmid < dhigh) return midTemp;
    return highTemp;
}


double ScheduleMethod::tempWithClosestPBad(double targetPBad, const multimap<double,double>& tempToPBad,
                double atLeast, double atMost) {
    
    if (tempToPBad.size() < 1) throw runtime_error("no entries in tempToPBad map");

    double minDiff = numeric_limits<double>::max();
    double bestTemp = -1;
    for (auto p : tempToPBad) {
        double temp = p.first;
        if ((atLeast == -1 or temp >= atLeast) and (atMost == -1 or temp <= atMost)) {
            double diff = abs(p.second-targetPBad);
            if (diff < minDiff) {
                minDiff = diff;
                bestTemp = temp;
            }
        }
    }

    if (bestTemp == -1) bestTemp = tempWithClosestPBad(targetPBad, tempToPBad, -1, -1);
    return bestTemp;
}

double ScheduleMethod::tempWithClosestPBad(double targetPBad, double atLeast, double atMost) const {
    return tempWithClosestPBad(targetPBad, tempToPBad, atLeast, atMost);
}

double ScheduleMethod::tempWithBestLRFit(double targetPBad, const multimap<double,double>& tempToPBad) {
    auto model = LinearRegression::bestFit(tempToPBad);    
    if (targetPBad <= model.goldilocksMinPBad)
        return tempWithClosestPBad(targetPBad, tempToPBad, -1, model.goldilocksMinPBad);
    if (targetPBad >= model.goldilocksMaxPBad)
        return tempWithClosestPBad(targetPBad, tempToPBad, model.goldilocksMaxPBad, -1);

    return model.interpolateWithinGoldilocks(targetPBad);
}

double ScheduleMethod::tempWithBestLRFit(double targetPBad) const {
    return tempWithBestLRFit(targetPBad, tempToPBad);
}

void ScheduleMethod::populatePBadCurve() {
    const double HIGH_PBAD_LIMIT = 0.99999;
    const double LOW_PBAD_LIMIT = 1e-10;
    double highTemp = doublingMethod(HIGH_PBAD_LIMIT, false);
    double lowTemp = doublingMethod(LOW_PBAD_LIMIT, true);
    double numSteps = pow(10, abs(log10(lowTemp)) + abs(log10(highTemp)));
    for (int T_i = 0; T_i <= log10(numSteps); T_i++) {
        double logTemp = log10(lowTemp) + T_i*(log10(highTemp)-log10(lowTemp))/log10(numSteps);
        double temp = pow(10, logTemp);

        getPBad(temp);
    }
}



void ScheduleMethod::printScheduleStatistics() {
    cout << "TInitial found in " << TInitialTime << "s for target pBad " << targetInitialPBad << ": " << endl;
    getPBad(TInitial);
    cout << "TFinal found in " << TFinalTime << "s for target pBad " << targetFinalPBad << ": " << endl;
    getPBad(TFinal);
    cout << "TDecay needed to traverse this range: " << -log(TFinal/TInitial) << endl;
    cout << endl;
}

double ScheduleMethod::totalTime() {
    return TInitialTime + TFinalTime;
}

int ScheduleMethod::totalSamples() {
    return TInitialSamples + TFinalSamples;
}
