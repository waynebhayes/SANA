#ifndef SPARSEMATRIX_H
#define SPARSEMATRIX_H

#include <unordered_map>
#include <vector>
#include <assert.h>
#include <math.h>
#include <iostream>

using namespace std;

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

    unsigned long size() const {
        return n;
    }

    T get(unsigned long node1, unsigned long node2) const {
        auto got = v[node1].find(node2);
        if (got == v[node1].end()) {
            return T();
        } else {
            return got->second;
        }
    }

    void set(T value, unsigned long node1, unsigned long node2) {
        v[node1][node2] = value;
    }

    template <class Archive>
    void serialize(Archive & archive) {
        archive(v, n);
    }
    
   /**
    * This function is part of the SparseMatrix library implemented by Petr Kessler
    *
    * Copyright (c) 2014 Petr Kessler (http://kesspess.1991.cz)
    *
    * @license  MIT
    * @link     https://github.com/uestla/Sparse-Matrix
    */
    SparseMatrix<T> multiply(const SparseMatrix<T> & m) const {
        if (size() != m.size()) {
            throw "Cannot multiply: Left matrix column count and right matrix row count don't match.";
        }

        SparseMatrix<T> result(size());

        T a;
        for (unsigned long j = 1; j <= size(); j++){
            for (unsigned long k = 1; k <= m.size(); k++){
                a = 0;
                if(m.get(k, j) != 0){
                    for (unsigned long i = 1; i <= size(); i++) {
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
    vector<unordered_map<unsigned long, T>> v;
};

#endif
