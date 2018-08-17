#ifndef PARETOMODE_HPP_
#define PARETOMODE_HPP_

#include "Mode.hpp"
#include "../Graph.hpp"
#include "../arguments/ArgumentParser.hpp"
#include "../measures/MeasureCombination.hpp"
#include "../methods/Method.hpp"
#include "../methods/SANA.hpp"

class ParetoMode : public Mode {
public:
    void run(ArgumentParser& args);
    std::string getName(void);

    static void createFolders();
    void setArgsForParetoMode(ArgumentParser& args);
    vector<Alignment> runParetoMode(Method *M, Graph *G1, Graph *G2);
    void printAlignments(vector<Alignment>& alignments);
    void printEdgeLists(Graph* G1, Graph* G2, vector<Alignment>& alignments);
};

#endif /* PARETOMODE_HPP_ */
