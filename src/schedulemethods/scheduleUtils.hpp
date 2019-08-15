#ifndef SCHEDULEUTILS_HPP
#define SCHEDULEUTILS_HPP

#include <vector>
#include <string>
#include <memory>

#include "ScheduleMethod.hpp"

using namespace std;

unique_ptr<ScheduleMethod> getScheduleMethod(string name);

struct ScheduleExpParams {
    vector<string> testedMethodNames;
    double targetInitialPBad;
    double targetFinalPBad;
    int runsPerMethod;
    ScheduleMethod::Resources maxResources;
    double sampleTime;
    double errorTol;
    int numValidationSamples;
};

void scheduleMethodComparison(SANA *const sana);

vector<string> formatMethodData(const ScheduleExpParams& params, string name, bool hasUnifiedRuntime,
                double TInitial, double TFinal, ScheduleMethod::Resources TIniRes,
                ScheduleMethod::Resources TFinRes, ScheduleMethod::Resources totalRes);

NormalDistribution getPBadDis(double temp, int numSamples, double sampleTime);

#endif