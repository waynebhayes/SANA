#include "randomSeed.hpp"
#include <random>

using namespace std;

random_device rd;
unsigned int currentSeed = rd();

void setSeed(unsigned int seed) {
    currentSeed = seed;
}

unsigned int getRandomSeed() {
    return currentSeed;
}

