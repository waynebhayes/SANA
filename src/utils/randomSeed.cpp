#include "randomSeed.hpp"
#include <random>
#include <unistd.h>

using namespace std;

random_device rd;
unsigned int currentSeed; // = rd() + gethostid() + time(0) + getpid();
static char doneInit = 0;

void setSeed(unsigned int seed) {
    currentSeed = seed;
    doneInit = 1;
}

unsigned int getRandomSeed() {
    if(!doneInit) {
    currentSeed = /*rd() +*/ gethostid() + time(0) + getpid(); // rd() fails on Jenkins.
    doneInit=1;
    }
    return currentSeed;
}

