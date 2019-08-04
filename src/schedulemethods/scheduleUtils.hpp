#ifndef SCHEDULEUTILS_HPP
#define SCHEDULEUTILS_HPP

#include <vector>
#include <string>
#include <memory>

#include "ScheduleMethod.hpp"

using namespace std;

unique_ptr<ScheduleMethod> getScheduleMethod(string name);

void scheduleMethodComparison(SANA *const sana);

vector<string> methodData(const unique_ptr<ScheduleMethod>& method, double maxTime,
                            int maxSamples, int numValidationSamples, double sampleTime);

NormalDistribution getPBadDis(double temp, int numSamples, double sampleTime);

#endif