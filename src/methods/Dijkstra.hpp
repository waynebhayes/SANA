#ifndef DIJKSTRA_HPP
#define DIJKSTRA_HPP
#include <vector>
#include "Method.hpp"
#include <random>
#include "../measures/localMeasures/LocalMeasure.hpp"
#include "../measures/Measure.hpp"
#include "../measures/MeasureCombination.hpp"
#include "../utils/randomSeed.hpp"

using namespace std;

class Dijkstra: public Method {

public:
	Dijkstra(Graph* G1, Graph* G2, MeasureCombination* MC);
    ~Dijkstra();

    Alignment run();
    void describeParameters(ostream& stream);
    string fileNameSuffix(const Alignment& A);

private:
    Graph* G1;
    Graph* G2;
    MeasureCombination* MC;

    uint n1;
    uint n2;

    vector<vector<bool> > G1AdjMatrix;
    vector<vector<bool> > G2AdjMatrix;
    vector<vector<ushort> > G1AdjLists;
    vector<vector<ushort> > G2AdjLists;

    vector<ushort> A;

   vector<vector<float> > sims;

   bool implementsLocking(){ return false; }
};

#endif
