#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "MeasureCombination.hpp"
#include "localMeasures/LocalMeasure.hpp"

MeasureCombination::MeasureCombination(){
}

MeasureCombination::~MeasureCombination() {
}

double MeasureCombination::eval(const Alignment& A) const {
    uint n = measures.size();
    double total = 0;
    for (uint i = 0; i < n; i++) {
        if (weights[i] > 0) total += measures[i]->eval(A) * weights[i];
    }
    return total;
}

double MeasureCombination::eval(const string& measureName, const Alignment& A) const {
    uint n = measures.size();
    for (uint i = 0; i < n; i++) {
        if (measures[i]->getName() == measureName) {
            return measures[i]->eval(A);
        }
    }
    throw runtime_error("Measure not found.");
    return 0;
}

void MeasureCombination::addMeasure(Measure* m, double weight) {
    measures.push_back(m);
    weights.push_back(weight);
}

void MeasureCombination::addMeasure(Measure* m) {
    measures.push_back(m);
    weights.push_back(0);
}

void MeasureCombination::printWeights(ostream& ofs) const {
    uint n = measures.size();
    for (uint i = 0; i < n; i++) {
        if (weights[i] > 0) {
            ofs << measures[i]->getName();
            ofs << ": " << weights[i] << endl;
        }
    }
}

void MeasureCombination::printMeasures(const Alignment& A, ostream& ofs) const {
    uint n = measures.size();
    for (uint i = 0; i < n; i++) {
        ofs << measures[i]->getName();
        ofs << ": " << measures[i]->eval(A) << endl;
    }
    uint count = 0;
    for (uint i = 0; i < n; i++) {
        if (weights[i] > 0) count++;
    }
    if (count > 1) {
        ofs << "Combined: " << eval(A) << " ( ";
        for (uint i = 0; i < n; i++) {
            if (weights[i] > 0) ofs << measures[i]->getName() << " " << weights[i] << " ";
        }
        ofs << ")" << endl;
    }
}

struct Crit {
    string name;
    double score;
    double weight;
};

bool critComp(Crit a, Crit b) {
    return a.weight > b.weight;
}

string MeasureCombination::toString() const {
    uint n = measures.size();
    vector<Crit> crits(0);
    Crit c;
    for (uint i = 0; i < n; i++) {
        if (weights[i] > 0) {
            c.name=measures[i]->getName();
            c.weight=weights[i];
            crits.push_back(c);
        }
    }
    sort(crits.begin(), crits.end(), critComp);
    string alig = "";
    for (uint i = 0; i < crits.size(); i++) {
        alig += crits[i].name + "_";
        if (crits[i].weight < 1) alig += extractDecimals(crits[i].weight, 2) + "_";
    }
    return alig.substr(0, alig.size()-1);;
}

void MeasureCombination::normalize() {
    normalizeWeights(weights);
}

double MeasureCombination::getWeight(const string& measureName) const {
    uint n = measures.size();
    for (uint i = 0; i < n; i++) {
        if (measures[i]->getName() == measureName) {
            return weights[i];
        }
    }
    throw runtime_error("Measure not found.");
}

Measure* MeasureCombination::getMeasure(const string& measureName) const {
    uint n = measures.size();
    for (uint i = 0; i < n; i++) {
        if (measures[i]->getName() == measureName) {
            return measures[i];
        }
    }
    throw runtime_error("Measure not found.");
}

Measure* MeasureCombination::getMeasure(int i) const {
    return measures[i];
}

bool MeasureCombination::containsMeasure(const string& measureName) {
    uint n = measures.size();
    for (uint i = 0; i < n; i++) {
        if (measures[i]->getName() == measureName) {
            return true;
        }
    }
    return false;
}

uint MeasureCombination::numMeasures() const {
    return measures.size();
}

void MeasureCombination::initn1n2(uint& n1, uint& n2) const {
    for (uint i = 0; i < numMeasures(); i++) {
        Measure* m = measures[i];
        if (m->isLocal()) {
            vector<vector<float> >* mSims = ((LocalMeasure*) m)->getSimMatrix();
            n1 = mSims->size();
            n2 = (*mSims)[0].size();
            return;
        }
    }
    throw runtime_error("There are no local measures");
}

vector<vector<float> > MeasureCombination::getAggregatedLocalSims() const {
    uint n1, n2;
    initn1n2(n1, n2);
    //combine all locals into a new local
    vector<vector<float> > combinedSims(n1, vector<float> (n2, 0));
    for (uint i = 0; i < numMeasures(); i++) {
        Measure* m = measures[i];
        double w = weights[i];
        if (m->isLocal() and w > 0) {
            vector<vector<float> >* mSims = ((LocalMeasure*) m)->getSimMatrix();
            for (uint i = 0; i < n1; i++) {
                for (uint j = 0; j < n2; j++) {
                    combinedSims[i][j] += w * (*mSims)[i][j];
                }
            }
        }
    }
    return combinedSims;
}

double MeasureCombination::getSumLocalWeight() const {
    double res = 0;
    for (uint i = 0; i < numMeasures(); i++) {
        if (measures[i]->isLocal()) {
            res += weights[i];
        }
    }
    return res;
}

void MeasureCombination::clearWeights() {
    for (uint i = 0; i < weights.size(); i++) {
        weights[i] = 0;
    }
}

void MeasureCombination::setWeight(const string& measureName, double weight) {
    uint n = measures.size();
    for (uint i = 0; i < n; i++) {
        if (measures[i]->getName() == measureName) {
            weights[i] = weight;
            return;
        }
    }
    throw runtime_error("Measure not found: "+measureName);
}
