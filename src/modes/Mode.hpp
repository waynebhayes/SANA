#ifndef MODE_HPP_
#define MODE_HPP_

#include "../arguments/ArgumentParser.hpp"

class Mode {
public:
	virtual void run(ArgumentParser& args) = 0;
	virtual ~Mode() {};
};

#endif /* MODE_HPP_ */
