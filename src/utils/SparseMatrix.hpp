#ifndef SPARSEMATRIX_H
#define SPARSEMATRIX_H

#include <vector>
#include <assert.h>
#include <math.h>
#include <iostream>

using namespace std;

class MySet {
public:
    typedef unsigned int SETTYPE;
    static const unsigned int setBits = sizeof(SETTYPE)*8;

    static int SIZE(unsigned int n)
    { return (n+setBits-1)/setBits; }   /* number of array elements needed to store n bits */

    MySet() {
        n = 0;
    }

    MySet(unsigned int n):
        array(SIZE(n)) {
        this->n = n;
    }

    unsigned int size() const {
        return n;
    }

    void init(unsigned int n) {
        this->n = n;
        array = vector<SETTYPE>(n);
    }

    void set(unsigned int element) {
        assert(element < n);
        array[element/setBits] |= (1UL << (element % setBits));
    }

    bool get(unsigned int element) const {
        assert(element < n);
        return array[element/setBits] & (1UL << (element % setBits)) ?
                true : false;
    }


		template <class Archive>
    void serialize(Archive & archive) {
        archive(array, n);
    }
private:
    unsigned int n;
    vector<SETTYPE> array;
};


class SparseSet {
public:
    SparseSet() {
        n = 0;
        sqrt_n = 0;
    }

    SparseSet(unsigned long n):
        sets(ceil(sqrt(n))) {
        this->n = n;
        sqrt_n = ceil(sqrt(n));
    }
    
    unsigned long size() const {
        return n;
    }

    void init(unsigned long n) {
        sqrt_n = ceil(sqrt(n));
        sets = vector<MySet>(sqrt_n);
        this->n = n;
    }

    void set(unsigned long element) {
        assert(element < n);
        int which = element / sqrt_n;
        if(sets[which].size() == 0) {
            sets[which].init(sqrt_n);
        }
        sets[which].set(element - which * sqrt_n);
    }

    bool get(unsigned long element) const {
        assert(element < n);
        int which = element / sqrt_n;
        return sets[which].size() != 0 && sets[which].get(element - which * sqrt_n);
    }

		template <class Archive>
    void serialize(Archive & archive) {
        archive(sets, n, sqrt_n);
    }

private:
    unsigned long n;
    unsigned int sqrt_n;
    vector<MySet> sets;
};

template <typename T>
class SparseMatrix {
public:
    SparseMatrix() {
        n = 0;
    }

    SparseMatrix(unsigned long n): 
        v(n) {
        this->n = n; 
    }

    SparseMatrix<T> & operator = (const SparseMatrix<T>& matrix) {
        if (&matrix != this) {
            n = matrix.n;
            v = matrix.v;            
        }

        return *this;
    }

    T get(int node1, int node2) const {
        if (v[node1].size() == 0) {
            return false;
        }

        return v[node1].get(node2);
    }

    unsigned long size() const {
        return n;
    }

    void set(int value, int node1, int node2) {
        if (v[node1].size() == 0) {
            v[node1].init(n);
        }

        v[node1].set(node2);
    }

    template <class Archive>
    void serialize(Archive & archive) {
        archive(v, n);
    }

    SparseMatrix<T> multiply(const SparseMatrix<T> & m) const {
        if (size() != m.size()) {
            throw "Cannot multiply: Left matrix column count and right matrix row count don't match.";
        }

        SparseMatrix<T> result(size());

        T a;
        for (int j = 1; j <= size(); j++){
            for (int k = 1; k <= m.size(); k++){
                a = 0;
                if(m.get(k, j) != 0){
                    for (int i = 1; i <= size(); i++) {
                        if(get(i, k) != 0 && m.get(k, j) != 0){
                            a += get(i, k) * m.get(k, j);
                            result.set(a, i, j);
                        }
                    }
                }
            }
        }
        return result;
    }

private:
    unsigned long n;
    vector<SparseSet> v;
};

#endif
