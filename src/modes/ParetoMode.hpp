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
    vector<Alignment>* runParetoMode(Method *M, Graph *G1, Graph *G2);
};

#endif /* PARETOMODE_HPP_ */
