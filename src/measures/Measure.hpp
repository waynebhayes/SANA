#ifndef MEASURE_HPP
#define MEASURE_HPP
#include <string>
#include <vector>
#include "../Graph.hpp"
#include "../utils/utils.hpp"
#include "../Alignment.hpp"
#include "../MultiAlignment.hpp"
#include "../utils/Timer.hpp"

class Measure {
public:

    Measure(Graph* G1, Graph* G2, string name);
    Measure(string name);
    Measure(vector<Graph>* GV, string name);
    virtual ~Measure();
    virtual double eval(const Alignment& A) =0;
    virtual double eval(const MultiAlignment& MA) =0;
    string getName();
    virtual bool isLocal();

protected:
    Graph* G1;
    Graph* G2;

private:
    string name;

};

#endif

