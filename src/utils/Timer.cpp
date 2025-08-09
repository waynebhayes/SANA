#include <sys/time.h>
#include <sys/resource.h>
#include <sstream>
#include <iomanip>
#include <ios>
#include <string>
#include <chrono>
#include <ctime>
#include "Timer.hpp"
using namespace std;

Timer::Timer() {}

void Timer::start() {
    startTime = get();
}

double Timer::elapsed() const {
    long long current;
    if (startTime == -1 or (current = get()) == -1) return -1;
    return (current-startTime) / 1000.0;
}

string Timer::elapsedString() const {
    ostringstream s;
    s << fixed << setprecision(3) << elapsed() << "s";
    return s.str();
}

long long Timer::get() {
    struct rusage usg;
    long long res = 0;
    if (getrusage(RUSAGE_SELF, &usg) == 0) {
        res += (usg.ru_utime.tv_sec + usg.ru_stime.tv_sec)*1000 +
               (usg.ru_utime.tv_usec + usg.ru_stime.tv_usec)/1000;
    } else {
        return -1;
    }
    if (getrusage(RUSAGE_CHILDREN, &usg) == 0) {
        res += (usg.ru_utime.tv_sec + usg.ru_stime.tv_sec)*1000 +
               (usg.ru_utime.tv_usec + usg.ru_stime.tv_usec)/1000;
    } else {
        return -1;
    }
    return res;
}

TimerTrue::TimerTrue() = default;

void TimerTrue::start() {
    startTime = get();
}

double TimerTrue::elapsed() const {
    const chrono::time_point<chrono::steady_clock> current = get();
    const chrono::duration<double> duration = current - startTime;
    return duration.count();
}

string TimerTrue::elapsedString() const {
    ostringstream s;
    s << fixed << setprecision(3) << elapsed() << "s";
    return s.str();
}

chrono::time_point<chrono::steady_clock> TimerTrue::get() {
    return chrono::steady_clock::now();
}