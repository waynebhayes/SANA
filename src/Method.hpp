#ifndef METHOD_HPP
#define	METHOD_HPP
#include <string>
#include "Graph.hpp"
#include "utils.hpp"
#include "Alignment.hpp"
#include <iostream>

class Method {
public:
    Method(Graph* G1, Graph* G2, string name);
    ~Method();
    Alignment runAndPrintTime();
    virtual Alignment run() =0;
    virtual void describeParameters(ostream& stream) =0;
    virtual string fileNameSuffix(const Alignment& A) =0;

    string getName();
    double getExecTime();
    
protected:
	Graph* G1;
	Graph* G2;
	
private:
	string name;
    double execTime;

};

#endif

