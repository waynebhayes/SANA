#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <random>

using namespace std;

class RandomNumberGenerator
{
public:
    RandomNumberGenerator();
    RandomNumberGenerator(const int& seed);
    
    void SetSeed(unsigned int seed);
    unsigned int GetSeed();

private:
    mt19937 numberGenerator;
    unsigned int currentSeed;
};

#endif /* RANDOM_HPP */
