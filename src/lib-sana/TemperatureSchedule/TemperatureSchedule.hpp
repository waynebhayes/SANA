#ifndef TEMPERATURE_SCHEDULE_H
#define TEMPERATURE_SCHEDULE_H

class TemperatureSchedule {

friend class TemperatureScheduleBuilder;

public:
    double GetTemperature(const double &progressFrac);    

    void SetTemperatureInitial(const double &initial);
    void SetTemperatureDecay(const double &decay);
private:
    TemperatureSchedule();
    ~TemperatureSchedule(){}

    void SearchTemperatureByLinearRegression();

    double tInitial;
    double tDecay;
};

#endif
