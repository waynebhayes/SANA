#ifndef DATABASE_HPP
#define DATABASE_HPP
#include "Graphette.hpp"
#include "Graph.hpp"
#include "utils/xrand.hpp"
#include <bits/stdc++.h>
#include <sys/resource.h>

typedef struct
{
    std::string canonicalPermutation;
    long canonicalDecimal;
}graphette;

class Database
{
public:
    Database();
    Database(short k);
    void addGraph(std::string filename, long long int numSamples);
    
private:
    ullint k_, numOrbitId_;
    int MAX_FD;
    std::string databaseDir;
    std::vector<graphette> g;
    std::vector<ullint> canonicalGraphette;
    std::vector<std::vector<ullint>> orbitId_; //orbit id of node j from ith canonical graphette
    Graphette* getCanonicalGraphette(Graphette* x);
};

#endif
