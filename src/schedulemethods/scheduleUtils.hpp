#ifndef SCHEDULEUTILS_HPP
#define SCHEDULEUTILS_HPP

#include <vector>
#include <string>
#include <memory>

#include "ScheduleMethod.hpp"

using namespace std;

unique_ptr<ScheduleMethod> getScheduleMethod(string name, SANA *const sana);

void scheduleMethodComparison(SANA *const sana);

vector<string> methodData(const unique_ptr<ScheduleMethod>& method, double maxTime, int maxSamples, double numSamples);


#endif