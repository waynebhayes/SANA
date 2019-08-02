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
    virtual string fileNameSuffix(const Alignment& A) =0;

    string getName();
    double getExecTime();

    void setLockFile(string fileName);
    void checkLockingBeforeReport(Alignment A);
    void checkNodeTypesBeforeReport(Alignment A);

protected:
    Graph* G1;
    Graph* G2;

    // whether or not we can use -lock for that method default to false
    virtual bool implementsLocking(){ return false; }
    string lockFileName = "";

private:
    string name;
    double execTime;
};

#endif
