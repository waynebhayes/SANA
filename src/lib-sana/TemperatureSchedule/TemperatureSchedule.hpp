#ifndef TEMPERATURE_SCHEDULE_H
#define TEMPERATURE_SCHEDULE_H

class TemeratureSchedule {

    friend class TemeratureScheduleBuilder;

    public:
        double getTemperature(double time);    

    private:
        TemeratureSchedule(){}
        ~TemeratureSchedule(){}

        double TInitial;
        double Tdecay;
        double TFinal;

        double timeFinal;
        bool useIterations = false;

};


#endif
