#ifndef SCHEDULEUTILS_HPP
#define SCHEDULEUTILS_HPP

#include <vector>
#include <string>
#include <memory>

#include "ScheduleMethod.hpp"

using namespace std;

unique_ptr<ScheduleMethod> getScheduleMethod(string name);

void scheduleMethodComparison(SANA *const sana);

vector<string> formatMethodData(string, double, double, double, double, double, int,
                                double, int, double, int, double, int, double);

NormalDistribution getPBadDis(double temp, int numSamples, double sampleTime);

#endif