#ifndef SEQUENCE_HPP
#define SEQUENCE_HPP
#include "LocalMeasure.hpp"
#include <unordered_map>
#include "../../arguments/graphLoader.hpp"

extern bool _graphsSwitched;

class Sequence: public LocalMeasure {
public:
    Sequence(Graph* G1, Graph* G2);
    virtual ~Sequence();
    void generateBitscoresFile(string bitscoresFile);

    static bool fulfillsPrereqs(Graph* G1, Graph* G2);

private:

    unordered_map<string,string> initNameMap(string curatedFastaFile);
    void initSimMatrix();

    static string blastScoreFile(const string& G1Name, const string& G2Name);
};

#endif

