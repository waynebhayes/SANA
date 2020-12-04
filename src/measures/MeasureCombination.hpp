#ifndef MEASURECOMBINATION_HPP
#define MEASURECOMBINATION_HPP

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>

#include "Measure.hpp"
#include "../MultiAlignment.hpp"

class MeasureCombination {
public:
    MeasureCombination();
    ~MeasureCombination();
    double eval(const Alignment& A) const;
    double evalMulti(const MultiAlignment& MA) const;
    double eval(const string& measureName, const Alignment& A) const;
    double evalMulti(const string& measureName, const MultiAlignment& MA) const;
    void addMeasure(Measure* m, double weight);
    void addMeasure(Measure* m);
    void printWeights(ostream& ofs) const;
    void printMeasures(const Alignment& A, ostream& ofs) const;
    double getWeight(const string& measureName) const;
    Measure* getMeasure(const string& measureName) const;
    Measure* getMeasure(int i) const;
    bool containsMeasure(const string& measureName);
    void normalize();
    uint numMeasures() const;
    string toString() const;

    double getSumLocalWeight() const;
    vector<vector<float> > getAggregatedLocalSims() const;
    vector<vector<vector<vector<float> > > > getAggregatedMultiLocalSims() const;
    int getNumberOfLocalMeasures() const;

private:
    vector<Measure*> measures;
    vector<double> weights;

    void initn1n2(uint& n1, uint& n2) const;
    void initnognon(uint& nog, uint& non) const;

    void clearWeights();
    void setWeight(const string& measureName, double weight);

};

#endif

