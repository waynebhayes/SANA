#include "PBadBinarySearch.hpp"

PBadBinarySearch::PBadBinarySearch(SANA *const sana) :
    ScheduleMethod(sana) {} 

double PBadBinarySearch::computeTempForPBad(double pBad) {
    return pBadBinarySearch(pBad);
}

void PBadBinarySearch::computeTInitial() {
    TInitial = computeTempForPBad(targetInitialPBad);
}

void PBadBinarySearch::computeTFinal() {
    TFinal = computeTempForPBad(targetFinalPBad);
}

