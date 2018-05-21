#include <math.h>
#include <unordered_map>
#include "TemperatureSchedule.hpp"

void TemperatureSchedule::SetTemperatureDecay(const double &decay) {
    tDecay = decay;
}

void TemperatureSchedule::SetTemperatureInitial(const double &initial) {
    tInitial = initial;
}

double TemperatureSchedule::GetTemperature(const double &progressFrac){
    return tInitial * exp(-tDecay * progressFrac);
}

void TemperatureSchedule::SearchTemperatureByLinearRegression() {
    //Hudson's implementation of searching for temperature will go here
}
