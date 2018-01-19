#include <math.h>

#include "TemeratureSchedule.hpp"


double TemeratureSchedule::getTemperature(double iter){
    double fraction = iter / TFinal;
    return TInitial * exp (-TDecay * fraction)
}