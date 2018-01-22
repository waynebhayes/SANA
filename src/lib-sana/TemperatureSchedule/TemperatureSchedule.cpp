#include <math.h>

#include "TemperatureSchedule.hpp"


double TemperatureSchedule::getTemperature(double iter){
    double fraction = iter / TFinal;
    return TInitial * exp (-TDecay * fraction);
}
