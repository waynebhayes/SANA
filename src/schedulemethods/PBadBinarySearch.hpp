#ifndef PBADBINARYSEARCH_HPP
#define PBADBINARYSEARCH_HPP

#include "ScheduleMethod.hpp"

class PBadBinarySearch : public ScheduleMethod {
public:
    PBadBinarySearch(SANA *const sana);

    static constexpr auto name = "pbad-binary-search";
    string getName() override { return name; }

    double computeTempForPBad(double targetPBad, double maxTime, int maxSamples) override;

};

#endif
