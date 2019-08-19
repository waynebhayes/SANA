//
// Created by taylor on 3/27/18.
//

#ifndef GOAVERAGE_HPP
#define GOAVERAGE_HPP
#include <Measure.hpp>

class GoAverage : public Measure {
public:
    GoAverage(Graph* G1,Graph* G2);
    virtual ~GoAverage();
    double eval(const Alignment &A);
};


#endif //SANA_GOAVERAGE_HPP
