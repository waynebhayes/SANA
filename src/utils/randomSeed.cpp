#include "randomSeed.hpp"
#include <random>
#include <unistd.h>

using namespace std;

random_device rd;
unsigned int currentSeed = rd() + gethostid() + time(0) + getpid();

void setSeed(unsigned int seed) {
    currentSeed = seed;
}

unsigned int getRandomSeed() {
    return currentSeed;
}

