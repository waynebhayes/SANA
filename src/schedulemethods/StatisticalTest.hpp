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
    StatisticalTest(SANA *const sana);

    static constexpr auto name = "statistical-test";
    string getName() override { return name; }

protected:
    void computeTInitial() override;
    void computeTFinal() override;

private:
    bool isRandomTemp(double temp, double highThresholdScore, double lowThresholdScore);
    double scoreForTemp(double temp);

    double expectedNumAccEInc(double temp, const vector<double>& EIncSample);

};

#endif
