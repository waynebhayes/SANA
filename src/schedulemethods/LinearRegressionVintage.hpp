#ifndef LINEARREGRESSIONVINTAGE_HPP
#define LINEARREGRESSIONVINTAGE_HPP

#include "LinearRegressionModern.hpp"

/* Original Linear Regression implementation */
class LinearRegressionVintage : public LinearRegressionModern {
public:
    LinearRegressionVintage(SANA *const sana);

    static constexpr auto name = "linear-regression-vintage";
    string getName() override { return name; }

protected:
    void computeBoth(double maxTime, int maxSamples) override;

};

#endif
