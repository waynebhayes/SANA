#ifndef LINEARREGRESSIONMODERN_HPP
#define LINEARREGRESSIONMODERN_HPP

#include "ScheduleMethod.hpp"

/* New Linear Regression implementation better integrated with the other schedule methods */
class LinearRegressionModern : public ScheduleMethod {
public:
    LinearRegressionModern(SANA *const sana);

    static constexpr auto name = "linear-regression-modern";
    string getName() override { return name; }

protected:
    void computeTInitial() override;
    void computeTFinal() override;

private:
    void computeBoth();


};



#endif

