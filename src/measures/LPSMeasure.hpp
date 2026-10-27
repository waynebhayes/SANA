#ifndef LPSMeasure_HPP
#define LPSMeasure_HPP
#include "Measure.hpp"

// mode: "l" -> LPS_L, "i" -> LPS_I, "s" -> LPS_S
class LPS : public Measure {
public:
    LPS(const Graph* G1, const Graph* G2, const std::string& mode);
    virtual ~LPS();
    double eval(const Alignment& A);
private:
    char mode;
    double omega; // number of node pairs of G1
    double e1; // |E1|
};
#endif