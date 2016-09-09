#ifndef LOCALMEASURE_HPP
#define LOCALMEASURE_HPP
#include "../Measure.hpp"

class LocalMeasure: public Measure {
public:
    LocalMeasure(Graph* G1, Graph* G2, string name);
    virtual ~LocalMeasure() =0;
    virtual double eval(const Alignment& A);
    bool isLocal();
    vector<vector<float> >* getSimMatrix();
    void writeSimsWithNames(string outfile);
    double balanceWeight();

protected:
    void loadBinSimMatrix(string simMatrixFileName);
    virtual void initSimMatrix() =0;
    
    vector<vector<float> > sims;
    static const string autogenMatricesFolder;
};

#endif

