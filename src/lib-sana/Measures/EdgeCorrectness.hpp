//
// Created by taylor on 3/21/18.
//

#ifndef SANA_EDGECORRECTNESS_HPP
#define SANA_EDGECORRECTNESS_HPP
#include "Measure.hpp"

class EdgeCorrectness: public Measure {
public:
    EdgeCorrectness(Graph* G1, Graph* G2);
    virtual ~EdgeCorrectness();
    double eval(const Alignment& A);

private:

};
#endif //SANA_EDGECORRECTNESS_HPP
