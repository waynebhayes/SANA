#ifndef SCHEDULEUTILS_HPP
#define SCHEDULEUTILS_HPP

#include <vector>
#include <string>
#include <memory>

#include "ScheduleMethod.hpp"

using namespace std;

unique_ptr<ScheduleMethod> getScheduleMethod(string name, SANA *const sana);

void scheduleMethodComparison(SANA *const sana, double targetInitialPBad, double targetFinalPBad);

void logMethodData(const unique_ptr<ScheduleMethod>& method, vector<vector<string>>& table);


#endif