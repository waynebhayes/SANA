#ifndef ANALYSISMODE_HPP_
#define ANALYSISMODE_HPP_

#include "Mode.hpp"
#include "../Graph.hpp"
#include "../arguments/ArgumentParser.hpp"

class AnalysisMode : public Mode {
public:
    void run(ArgumentParser& args);
    std::string getName(void);
};

#endif /* ANALYSISMODE_HPP_ */
