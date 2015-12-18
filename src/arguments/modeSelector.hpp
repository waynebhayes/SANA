#ifndef MODESELECTOR_HPP_
#define MODESELECTOR_HPP_

#include "ArgumentParser.hpp"
#include "../modes/Mode.hpp"

//returns whether there exists a mode with name 'name'
bool validMode(string name);

Mode* selectMode(ArgumentParser& args);

#endif /* MODESELECTOR_HPP_ */
