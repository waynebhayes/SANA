#ifndef XRAND_HPP
#define XRAND_HPP

#include <random>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <cstdlib>

uint xrand(uint begin, uint end); //the range is [begin, end)
void xshuffle(std::vector<uint>& nodes, uint len);
#endif