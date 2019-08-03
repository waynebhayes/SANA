#ifndef LINEARREGRESSIONMODERN_HPP
#define LINEARREGRESSIONMODERN_HPP

#include "ScheduleMethod.hpp"

/* New Linear Regression implementation better integrated with the other schedule methods */
class LinearRegressionModern : public ScheduleMethod {
public:
    LinearRegressionModern(SANA *const sana);

    static constexpr auto name = "linear-regression-modern";
    virtual string getName() override { return name; }

    void setTargetInitialPBad(double pBad) override; 
    void setTargetFinalPBad(double pBad) override;
protected:
    void vComputeTInitial(double maxTime, int maxSamples) override;
    void vComputeTFinal(double maxTime, int maxSamples) override;

    virtual void computeBoth(double maxTime, int maxSamples);

private:
    bool alreadyComputed;


};



#endif

