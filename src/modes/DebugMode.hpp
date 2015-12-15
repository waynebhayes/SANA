#ifndef DEBUGMODE_HPP_
#define DEBUGMODE_HPP_

#include "Mode.hpp"
#include "../Graph.hpp"
#include "../arguments/ArgumentParser.hpp"

class DebugMode : public Mode {
public:
	void run(ArgumentParser& args);
};

#endif /* DEBUGMODE_HPP_ */
