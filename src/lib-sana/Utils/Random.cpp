#include "Random.hpp"
#include <random>
#include <unistd.h>

using namespace std;

RandomNumberGenerator::RandomNumberGenerator() {
    SetSeed(gethostid() + time(0) + getpid());
}

RandomNumberGenerator::RandomNumberGenerator(const int& seed) {
    SetSeed(seed);
}

void RandomNumberGenerator::SetSeed(unsigned int seed) {
    currentSeed = seed;
    numberGenerator.seed(currentSeed);
}

unsigned int RandomNumberGenerator::GetSeed() {
    return currentSeed;
}



