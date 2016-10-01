#ifndef ARGUMENTDESCRIPTIONS_HPP
#define ARGUMENTDESCRIPTIONS_HPP
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <iomanip>
#include <sstream>
#include <fstream>

using namespace std;

extern vector<array<string, 5>> argumentDescriptions;

void printAllArgumentDescriptions();
string printItem(const array<string, 5> &item);
string formatDescription(string description);
#endif
