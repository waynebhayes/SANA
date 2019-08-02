#ifndef ITERATEDAMEUR_HPP
#define ITERATEDAMEUR_HPP

#include <vector>

#include "ScheduleMethod.hpp"

using namespace std;

/* applies the method iteratively to converge to a better guess */
class IteratedAmeur : public ScheduleMethod {
public:
    IteratedAmeur(SANA *const sana);

    static constexpr auto name = "iterated-ameur";
    string getName() override { return name; }

    double computeTempForPBad(double pBad) override;
    
protected:
    void computeTInitial() override;
    void computeTFinal() override;

private:

};

#endif
