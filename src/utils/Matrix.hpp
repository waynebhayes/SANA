#ifndef MATRIX_HPP
#define MATRIX_HPP

#include "utils.hpp"
#include "SparseMatrix.hpp"
#include <vector>

using namespace std;
typedef unsigned char uchar;

#ifdef MULTI_PAIRWISE
    #define MATRIX_UNIT uchar // uchar allows 255 networks, ushort allows up to 65535 of them
#else
    #define MATRIX_UNIT bool
#endif

#ifdef SPARSE
    #define INNER_CONTAINER unordered_map<uint, T>
    #define MATRIX_DATA_STRUCTURE SparseMatrix<T>
#else
    #define INNER_CONTAINER vector<T>
    #define MATRIX_DATA_STRUCTURE vector<vector<T> >
#endif

template <typename T>
class Matrix {
public:
    Matrix();
    Matrix(const Matrix & matrix);
    Matrix(uint numberOfNodes);
    Matrix(uint row, uint col);

    Matrix & operator = (const Matrix & matrix);
    INNER_CONTAINER & operator [] (uint node1);

    const T get(uint node1, uint node2) const;
    void reserve_inner(unordered_map<uint,int> neighborCount);
    uint size() const;

    template <class Archive>
    void serialize(Archive & archive) {
        archive(CEREAL_NVP(data));
    }

private:
   MATRIX_DATA_STRUCTURE data;
};


template <typename T>
inline INNER_CONTAINER & Matrix<T>::operator [] (uint node1) {
    return data[node1];
}

template <typename T>
Matrix<T>::Matrix(uint row, uint col) {
#ifdef SPARSE
    data = MATRIX_DATA_STRUCTURE(row);
#else
    data = MATRIX_DATA_STRUCTURE(row,
           vector<T>(col, T()));
#endif
}

template <typename T>
inline const T Matrix<T>::get(uint node1, uint node2) const {
#ifdef SPARSE
    return data.get(node1, node2);
#else
    return data[node1][node2];
#endif
}

template <typename T>
void Matrix<T>::reserve_inner(unordered_map<uint,int> neighborCount) {
    for(auto p : neighborCount){
        data[p.first].reserve(p.second);
    }
}

template <typename T>
Matrix<T>::Matrix() {
}

template <typename T>
Matrix<T>::Matrix(const Matrix & matrix) {
    data = matrix.data;
}

template <typename T>
Matrix<T>::Matrix(uint numberOfNodes) {
#ifdef SPARSE
    data = MATRIX_DATA_STRUCTURE(numberOfNodes);
#else
    data = MATRIX_DATA_STRUCTURE(numberOfNodes,
           vector<T>(numberOfNodes, T()));
#endif
}

template <typename T>
Matrix<T> & Matrix<T>::operator = (const Matrix & matrix) {
    data = matrix.data;
    return *this;
}

template <typename T>
uint Matrix<T>::size() const {
    return data.size();
}
#endif
