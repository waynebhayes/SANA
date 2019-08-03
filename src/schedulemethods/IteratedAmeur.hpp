#ifndef ITERATEDAMEUR_HPP
#define ITERATEDAMEUR_HPP

#include <vector>

#include "Ameur.hpp"

using namespace std;

/* applies the method iteratively to converge to a better guess */
class IteratedAmeur : public Ameur {
public:
    IteratedAmeur(SANA *const sana);

    static constexpr auto name = "iterated-ameur";
    string getName() override { return name; }

    double computeTempForPBad(double targetPBad, double maxTime, int maxSamples) override;
    
};

#endif
