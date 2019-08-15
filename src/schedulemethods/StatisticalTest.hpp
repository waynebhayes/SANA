#ifndef STATISTICALTEST_HPP
#define STATISTICALTEST_HPP

#include <vector>

#include "ScheduleMethod.hpp"

using namespace std;

/* Nil's old statistical-test based method.
NOTE: It does NOT target specific pBads.
Instead, it finds start and end temps using other criteria */
class StatisticalTest : public ScheduleMethod {
public:
    StatisticalTest() =default;

    static constexpr auto NAME = "statistical-test";
    string getName() override { return NAME; }

protected:
    void vComputeTInitial(Resources maxRes) override;
    void vComputeTFinal(Resources maxRes) override;

private:
    bool isRandomTemp(double temp, double highThresholdScore, double lowThresholdScore);
    double scoreForTemp(double temp);

    double expectedNumAccEInc(double temp, const vector<double>& EIncSample);

};

#endif
