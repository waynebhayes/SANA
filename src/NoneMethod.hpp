#ifndef NONEMETHOD_HPP
#define	NONEMETHOD_HPP
#include <string>
#include <iostream>
#include "Method.hpp"
#include "Graph.hpp"
#include "utils.hpp"
#include "Alignment.hpp"
#include <iostream>

class NoneMethod : public Method {
public:
    NoneMethod(Graph* G1, Graph* G2, string startAName);
    ~NoneMethod();
    Alignment run();
    void describeParameters(ostream& stream);
    string fileNameSuffix(const Alignment& A);
private:
    Alignment A;
};

#endif

