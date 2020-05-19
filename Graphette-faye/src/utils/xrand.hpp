#ifndef XRAND_HPP
#define XRAND_HPP
#include <bits/stdc++.h>

typedef unsigned long long  ullint;

ullint xrand(ullint begin, ullint end); //the range is [begin, end)
void xshuffle(std::vector<ullint>& nodes, ullint len);
#endif