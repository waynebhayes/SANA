#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <cmath>
#include <vector>
#include <type_traits>

#include "Misc.hpp"
#include "Stats.hpp"

template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
class CircularBuffer {
public:
    explicit CircularBuffer(size_t size):
        full(false), use_default(false), default_value(0), sum(0), head(0), data(size) {}

    CircularBuffer(size_t size, T default_fill):
        full(true), use_default(true), default_value(default_fill), sum(default_fill * size), head(0), data(size) {}

    virtual ~CircularBuffer() = default;

    bool isFull() const { return full; }
    size_t capacity() const {return data.size();}
    size_t size() const {return full ? data.size() : head;}

    virtual void resetBuffer() {
        head = 0;
        if (!use_default) {
            sum = 0;
            full = false; // The old data does not have to be zeroed because it will just be ignored if not full.
        }
        else {
            sum = default_value * data.size();
            std::fill(data.begin(), data.end(), default_value); // Buffer must be filled in with its default value.
            full = true;
        }
    };

    virtual void insert(T newValue) {
        if (full) {
            sum -= data[head];
        }
        sum += newValue;
        data[head++] = newValue;

        if (head >= data.size()) {
            full = true;
            head = 0;
        }
    }

    virtual void recalculateStats() {
        sum = 0;
        const size_t count = size();
        for (size_t i = 0; i < count; ++i) {sum += data[i];}
    }

    double quickAverage() const {
        const size_t count = size();
        return count == 0 ? 0.0 : static_cast<double>(sum) / count;
    }

    double accurateAverage() {
        recalculateStats();
        return quickAverage();
    }

    // TODO: This bad and borrowed from SANA 2. This is just not a great equilibrium, it should be fixed. -ML
    bool trendingUpwards() const {
        const size_t count = size();
        if (count < 2) return false;

        int trend = 0;
        T lastScore = data[0];
        for (size_t i = 1; i < count; ++i) {
            if (data[i] > lastScore) ++trend;
            else if (data[i] < lastScore) --trend;
            lastScore = data[i];
        }
        return trend > 0;
    }

protected:
    bool full;
    bool use_default;
    T default_value;
    T sum;

    size_t head;
    vector<T> data;
};

template<typename T, typename = typename enable_if<is_arithmetic<T>::value>::type>
class CircularSTATBuffer final : public CircularBuffer<T> {
public:
    explicit CircularSTATBuffer(size_t size):
        CircularBuffer<T>(size), sumSquared(0) {}

    CircularSTATBuffer(size_t size, T default_fill):
        CircularBuffer<T>(size, default_fill), sumSquared(default_fill * default_fill * size) {}

    ~CircularSTATBuffer() override = default;

    void insert(T newValue) override {
        if (this->full) {
            T oldValue = this->data[this->head];
            this->sum -= oldValue;
            sumSquared -= oldValue * oldValue;
        }
        this->sum += newValue;
        sumSquared += newValue * newValue;

        this->data[this->head++] = newValue;

        if (this->head >= this->data.size()) {
            this->head = 0;
            this->full = true;
        }
    }

    void resetBuffer() override {
        CircularBuffer<T>::resetBuffer();
        this->sumSquared = this->use_default ? (this->default_val * this->default_val * this->data.size()) : 0;
    }

    void recalculateStats() override {
        this->sum = 0;
        sumSquared = 0;
        const size_t count = this->size();
        for (size_t i = 0; i < count; ++i) {
            T val = this->data[i];
            this->sum += val;
            this->sumSquared += val * val;
        }
    }

    double variance() {
        return variance(this->size());
    }

    double quickConfidenceInterval(const double confidence) const {
        const size_t n = this->size();
        if (n < 2) return 0.0;
        return StatTDistP2Z((1.0 - confidence) / 2.0, n - 1) * std::sqrt(variance(n) / n);
    }

    double accurateConfidenceInterval(const double confidence) {
        recalculateStats();
        return quickConfidenceInterval(confidence);
    }

protected:
    double variance(const size_t n) {
        if (n < 2) return 0.0;
        const double meanSum = static_cast<double>(this->sum);
        const double numerator = static_cast<double>(this->sumSquared) - (meanSum * meanSum / n);
        return std::fabs(numerator) / (n - 1);
    }

    T sumSquared;
};


#endif //CIRCULARBUFFER_H