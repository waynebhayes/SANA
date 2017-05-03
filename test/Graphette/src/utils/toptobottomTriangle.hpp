#ifndef TOPTOBOTTOMTRIANGLE_HPP
#define TOPTOBOTTOMTRIANGLE_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include "../HalfMatrix.hpp"

void convertToptoBottomTriangle(uint k);
uint convertNum(uint k, uint n);
std::string createPermutation(std::string perm1, std::string perm2); //Creates a permutation from first argument to second argument
#endif