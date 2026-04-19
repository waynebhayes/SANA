#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <cmath>
#include <vector>
#include <type_traits>

#include "Misc.hpp"
#include "Stats.hpp"
#include "../methods/SANAThree.hpp"

using namespace std;

template<typename T, typename = typename enable_if<is_arithmetic<T>::value>::type>
class CircularBuffer {
public:
    explicit CircularBuffer(size_t size) {
        full = false;
        default_fill = 0;
        sum = 0;
        data = vector<T>(size);
        currentPlace = data.begin();
    }

    CircularBuffer(size_t size, T default_fill) {
        full = true;
        sum = default_fill * size;
        this->default_fill = default_fill;
        data = vector<T>(size, default_fill);
        currentPlace = data.begin();
    }

    virtual ~CircularBuffer() = default;

    bool isFull() const { return full; }

    unsigned bufferSize() const {return data.end() - data.begin();}

    virtual void resetBuffer() {
        if (default_fill == 0) {
            sum = 0;
            full = false; // The old data does not have to be zeroed because it will just be ignored if not full.
        }
        else {
            sum = default_fill * data.size();
            fill(data.begin(), data.end(), default_fill); // Buffer must be filled in with its default value.
        }
        currentPlace = data.begin();
    };

    virtual void insert(T newValue) {
        if (full) {
            sum -= *currentPlace;
            sum += newValue;
            *currentPlace++ = newValue;
        }
        else {
            sum += newValue;
            *currentPlace++ = newValue;
        }
        if (currentPlace >= data.end()) {
            full = true;
            currentPlace = data.begin();
        }
    }

    virtual void recalculateStats() {
        sum = 0;
        if (full)
            for (T i: data) sum += i;
        else
            for (auto iter = data.begin(); iter < currentPlace; ++iter) sum += *iter;
    }

    double quickAverage() const {
        return full ? sum / data.size() : static_cast<double>(sum) / (currentPlace - data.begin());
    }

    double accurateAverage() {
        recalculateStats();
        return quickAverage();
    }

    // TODO: This bad and borrowed from SANA 2. This is just not a great equilibrium, it should be fixed. -ML
    bool trendingUpwards() const {
        int trend = 0;
        auto end = full ? data.end() : currentPlace;
        T lastScore = *data.begin();
        for (auto iter = data.begin() + 1; iter < end; ++iter) {
            if (*iter > lastScore) trend++;
            else if (*iter < lastScore) trend--;
            lastScore = *iter;
        }
        return trend > 0;
    }

    unsigned size() const {
        return full ? data.size(): currentPlace - data.begin();
    }

protected:
    bool full;
    T sum;
    T default_fill;

    vector<T> data;
    typename vector<T>::iterator currentPlace;
};

template<typename T, typename = typename enable_if<is_arithmetic<T>::value>::type>
class CircularSTATBuffer final : public CircularBuffer<T> {
public:
    explicit CircularSTATBuffer(size_t size):
        CircularBuffer<T>(size) {
        sumSquared = 0;
    }

    CircularSTATBuffer(size_t size, T default_fill):
        CircularBuffer<T>(size, default_fill) {
        sumSquared = default_fill * default_fill * size;
    }

    ~CircularSTATBuffer() override = default;

    void insert(T newValue) override {
        if (this->full) {
            T oldValue = *this->currentPlace;
            this->sum -= oldValue;
            this->sum += newValue;
            sumSquared -= oldValue * oldValue;
            sumSquared += newValue * newValue;
            *this->currentPlace = newValue;
            ++this->currentPlace;
        }
        else {
            this->sum += newValue;
            sumSquared += newValue * newValue;
            *this->currentPlace++ = newValue;
        }
        if (this->currentPlace == this->data.end()) {
            this->full = true;
            this->currentPlace = this->data.begin();
        }
    }

    void resetBuffer() override {
        CircularBuffer<T>::resetBuffer();
        if (this->default_fill == 0) sumSquared = 0;
        else {
            sumSquared = this->default_fill * this->default_fill * this->data.size();
        }
    }

    void recalculateStats() override {
        this->sum = 0;
        sumSquared = 0;
        if (this->full)
            for (T i: this->data) {
                this->sum += i;
                this->sumSquared += i * i;
            }
        else
            for (auto iter = this->data.begin(); iter < this->currentPlace; ++iter) {
                this->sum += *iter;
                sumSquared += *iter * *iter;
            }
    }

    double variance() {
        unsigned n;
        if (this->full) n = this->data.size();
        else n = this->currentPlace - this->data.begin();

        return fabs(static_cast<double>(sumSquared - this->sum*this->sum) / n) / (n - 1);
    }

    double quickConfidenceInterval(const double confidence) const {
        unsigned n;
        if (this->full) n = this->data.size();
        else n = this->currentPlace - this->data.begin();

        return StatTDistP2Z((1.-confidence)/2, n - 1) * sqrt(variance(n) / n);
    }

    double accurateConfidenceInterval(const double confidence) {
        recalculateStats();
        return quickConfidenceInterval(confidence);
    }

protected:
    double variance(const unsigned n) {
        return fabs(static_cast<double>(sumSquared - this->sum*this->sum) / n) / (n - 1);
    }

    T sumSquared;
};


#endif //CIRCULARBUFFER_H