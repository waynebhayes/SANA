#include <sys/time.h>
#include <sys/resource.h>
#include <sstream>
#include <iomanip>
#include <ios>
#include <string>
#include "Timer.hpp"
using namespace std;

Timer::Timer() {}

void Timer::start() {
    startTime = get();
}

double Timer::elapsed() const {
    if (startTime == -1)
       return -1;

    long long currentTime = get();
    if (currentTime == -1)
       return -1;

    return (currentTime - startTime) / 1000.0;
}

string Timer::elapsedString() const {
    ostringstream s;
    s << fixed << setprecision(3) << elapsed() << "s";
    return s.str();
}

long long Timer::get() {
    struct rusage usg;
    bool fail = false;
    long long res = 0;
    if (getrusage(RUSAGE_SELF, &usg) == 0) {
        res += (usg.ru_utime.tv_sec + usg.ru_stime.tv_sec) * 1000;
        res += (usg.ru_utime.tv_usec + usg.ru_stime.tv_usec) / 1000;
    } else {
        fail = true;
    }

    if (getrusage(RUSAGE_CHILDREN, &usg) == 0) {
        res += (usg.ru_utime.tv_sec + usg.ru_stime.tv_sec) * 1000;
        res += (usg.ru_utime.tv_usec + usg.ru_stime.tv_usec) / 1000;
    } else {
        fail = true;
    }
    return fail ? -1 : res;
}
