//
// Created by Viet on 5/28/2018.
//

#ifndef SANA_MEASURE_H
#define SANA_MEASURE_H
#include <string>

class Measure {
    std::string name;
public:
    Measure(std::string name);
    virtual ~Measure();

    virtual double eval(Graph* G1, Graph* G2, Alignment& A) const = 0;
    virtual bool isLocal() const;
    virtual double balanceWeight() {return 0;};
    virtual vector<ushort> getMappingforNC() const {return vector<ushort>{0};}

    std::string getName() const;
};
#endif //SANA_MEASURE_H