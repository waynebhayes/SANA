#ifndef TEMPERATURE_SCHEDULE_H
#define TEMPERATURE_SCHEDULE_H

class TemperatureSchedule {

    friend class TemperatureScheduleBuilder;

    public:
        double getTemperature(double time);    

    private:
        TemperatureSchedule(){}
        ~TemperatureSchedule(){}

        double TInitial;
        double TDecay;
        double TFinal;

        double timeFinal;
        bool useIterations = false;

};


#endif
