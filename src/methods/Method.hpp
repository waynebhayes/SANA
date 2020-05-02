#ifndef METHOD_HPP
#define METHOD_HPP
#include <string>
#include <iostream>
#include "../Graph.hpp"
#include "../utils/utils.hpp"
#include "../Alignment.hpp"
#include "../utils/Timer.hpp"

class Method {
public:
    Method(Graph* G1, Graph* G2, string name);
    virtual ~Method();
    Alignment runAndPrintTime();
    virtual Alignment run() =0;
    virtual void describeParameters(ostream& stream) =0;
    virtual string fileNameSuffix(const Alignment& A) const =0;

    string getName() const;
    double getExecTime();

protected:
    Graph* G1;
    Graph* G2;

private:
    string name;
    double execTime;
};

#endif
