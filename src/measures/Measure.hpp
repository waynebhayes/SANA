#ifndef MEASURE_HPP
#define MEASURE_HPP
#include <string>
#include "../Graph.hpp"
#include "../utils/utils.hpp"
#include "../Alignment.hpp"
#include "../utils/Timer.hpp"

class Measure {
public:

    Measure(Graph* G1, Graph* G2, string name);
    virtual ~Measure();
    virtual double eval(const Alignment& A) =0;
    string getName();
    virtual bool isLocal();
    virtual double balanceWeight() {return 0;};
    
    virtual vector<uint> getMappingforNC() const {return vector<uint>{0};}
protected:
    Graph* G1;
    Graph* G2;

private:
    string name;

};

#endif

