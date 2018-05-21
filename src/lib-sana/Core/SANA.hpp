#ifndef SANA_H
#define SANA_H

#include "SANAConfiguration.hpp"
#include "SANAResult.hpp"
#include "Graph.hpp"
#include <vector>

using namespace std;

class SANA {
public:
    SANA ();
    SANA (const SANAConfiguration &);
    SANAResult StartAligner(const vector<Graph> &networkList);
private:
    SANAConfiguration config;
};

#endif
