//
// Created by taylor on 3/21/18.
//

#ifndef SANA_MEASURE_H
#define SANA_MEASURE_H

#include "Alignment.hpp"
#include "Graph.hpp"


class Measure {
    public:
        Measure() : G1(nullptr),G2(nullptr),name("") {}
        Measure(Graph* G1, Graph* G2, string name);
        virtual ~Measure();
        virtual double eval(const Alignment& A) = 0;
        string getName();
        virtual bool isLocal();
        virtual double balanceWeight() { return 0; }

        virtual vector <ushort> getMappingforNC() const { return vector<ushort>{0}; }

    protected:
        Graph* G1;
        Graph* G2;

    private:
        string name;
};


#endif //SANA_MEASURE_H
