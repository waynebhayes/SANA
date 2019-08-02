#ifndef LINEARREGRESSIONVINTAGE_HPP
#define LINEARREGRESSIONVINTAGE_HPP

#include "ScheduleMethod.hpp"

/* Original Linear Regression implementation */
class LinearRegressionVintage : public ScheduleMethod {
public:
    LinearRegressionVintage(SANA *const sana);

    static constexpr auto name = "linear-regression-vintage";
    string getName() override { return name; }

protected:
    void computeTInitial() override;
    void computeTFinal() override;

private:
    void computeBoth();

};

#endif
