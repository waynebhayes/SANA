#ifndef TIMER_HPP
#define TIMER_HPP
#include <string>
#include <chrono>
using namespace std;

class Timer {
public:
    Timer();

    void start();
    double elapsed() const;
    string elapsedString() const;

private:
    long long startTime;
    static long long get();
};

class TimerTrue {
public:
    TimerTrue();

    void start();
    double elapsed() const;
    string elapsedString() const;

private:
    chrono::time_point<chrono::steady_clock> startTime;
    static chrono::time_point<chrono::steady_clock> get();
};



#endif
