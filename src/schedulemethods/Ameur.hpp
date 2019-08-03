#ifndef AMEUR_HPP
#define AMEUR_HPP

#include <vector>

#include "ScheduleMethod.hpp"

using namespace std;

/* Method from the paper "computing the initial temperature of simulated annealing" 
by Walid Ben-Ameur et al */
class Ameur : public ScheduleMethod {
public:
    Ameur(SANA *const sana);

    static constexpr auto NAME = "ameur-method";
    virtual string getName() override { return NAME; }

    virtual double computeTempForPBad(double targetPBad, double maxTime, int maxSamples) override;
    
protected:

    vector<double> getEIncSample(double temp, int sampleSize, double& resPBad);
    double computeTempForEIncs(double targetPBad, double startTempGuess, vector<double> EIncs);

};

#endif
