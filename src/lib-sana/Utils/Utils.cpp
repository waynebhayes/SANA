#include <random>
#include "Random.hpp"
#include "Utils.hpp"

using namespace std;

RandomNumberGenerator randObj;
mt19937 gen(randObj.GetSeed());
ranlux24_base fastGen(randObj.GetSeed());

int Utils::RandInt(int low, int high) {
    uniform_int_distribution <> dis(low,high);
    return dis(fastGen);
}

