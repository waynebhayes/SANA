#ifndef GENERICLOCALMEASURE_HPP
#define GENERICLOCALMEASURE_HPP
#include "LocalMeasure.hpp"

class GenericLocalMeasure: public LocalMeasure {
public:
    GenericLocalMeasure(Graph* G1, Graph* G2, string name, const vector<vector<float> >& simMatrix);
    virtual ~GenericLocalMeasure();
private:

    void initSimMatrix();
};

#endif

