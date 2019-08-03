#include "PBadBinarySearch.hpp"

PBadBinarySearch::PBadBinarySearch(SANA *const sana) :
    ScheduleMethod(sana) {} 

double PBadBinarySearch::computeTempForPBad(double targetPBad, double maxTime, int maxSamples) {
    return pBadBinarySearch(targetPBad, maxTime, maxSamples);
}

