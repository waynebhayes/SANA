#ifndef NODECORRECTNESS_HPP
#define NODECORRECTNESS_HPP
#include <vector>
#include "Measure.hpp"

class NodeCorrectness: public Measure {
public:
    NodeCorrectness(const Alignment& A);
    virtual ~NodeCorrectness();
    double eval(const Alignment& A);
    
    static vector<ushort> convertAlign(const Graph& G1, const Graph& G2, const vector<string>& E);    
	
    static bool fulfillsPrereqs(Graph* G1, Graph* G2);
    
private:
    Alignment trueA;

};

#endif

