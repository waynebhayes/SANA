#ifndef NORMALMODE_HPP_
#define NORMALMODE_HPP_

#include "Mode.hpp"
#include "../Graph.hpp"
#include "../arguments/ArgumentParser.hpp"
#include "../measures/MeasureCombination.hpp"
#include "../methods/Method.hpp"

class NormalMode : public Mode {
public:
    void run(ArgumentParser& args);
    std::string getName(void);

    static void createFolders();
};

#endif /* NORMALMODE_HPP_ */
