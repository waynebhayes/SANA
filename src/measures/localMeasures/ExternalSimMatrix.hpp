#ifndef EXTERNALSIMMATRIX_HPP
#define EXTERNALSIMMATRIX_HPP
#include "LocalMeasure.hpp"

class ExternalSimMatrix: public LocalMeasure {
public:
    ExternalSimMatrix(Graph* G1, Graph* G2, string file, int format);
    virtual ~ExternalSimMatrix();
private:
    void initSimMatrix();
    void loadFormat0();
    void loadFormat1();
    string file;
    int format;
};

#endif

