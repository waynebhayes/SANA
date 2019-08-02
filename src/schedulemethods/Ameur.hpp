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

    static constexpr auto name = "ameur-method";
    string getName() override { return name; }

    double computeTempForPBad(double pBad) override;
    
    friend class IteratedAmeur;
    
protected:
    void computeTInitial() override;
    void computeTFinal() override;

private:

    vector<double> getEIncSample(double temp, int sampleSize, double& resPBad);
    double computeTempForPBad(double targetPBad, double startTempGuess, vector<double> EIncs);

    static double tolerance(double targetPBad);


};

#endif
