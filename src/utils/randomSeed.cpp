#include "randomSeed.hpp"
#include <random>
#include <unistd.h>

using namespace std;

//this should be refactored without global variables -Nil
unsigned int currentSeed;
static bool doneInit = false;

void setSeed(unsigned int seed) {
	currentSeed = seed;
	doneInit = true;
}

void setRandomSeed() {
	currentSeed = gethostid() + time(0) + getpid();
	// random_device rd;
	// currentSeed += rd(); //rd() fails on Jenkins.
	doneInit = true;
}

unsigned int getRandomSeed() {
	if (not doneInit) setRandomSeed();
	return currentSeed;
}
