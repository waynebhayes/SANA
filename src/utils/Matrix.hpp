#ifndef MATRIX_HPP
#define MATRIX_HPP

#include "utils.hpp"
#include "SparseMatrix.hpp"
#include <vector>

using namespace std;

#ifdef WEIGHTED
    #define WEIGHTED_VALUE ushort
#else
    #define WEIGHTED_VALUE bool
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

    T get(uint node1, uint node2) const;
    void set(T value, uint node1, uint node2);

    uint size() const;

    void connect(T value, uint node1, uint node2);
    bool isConnected(uint node1, uint node2) const;

    template <class Archive>
    void serialize(Archive & archive) {
        archive(CEREAL_NVP(data));
    }

private:
   MATRIX_DATA_STRUCTURE data;
};

/*
 * The inline prefix is a must (mostly for get), otherwise it will 
 * fail the speed test. And these inline functions must be defined in 
 * the header files.
 */


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
inline T Matrix<T>::get(uint node1, uint node2) const {
#ifdef SPARSE
    return data.get(node1, node2);
#else
    return data[node1][node2];
#endif
}

template <typename T>
inline void Matrix<T>::set(T value, uint node1, uint node2) {
#ifdef SPARSE
    if (data.get(node1, node2) || data.get(node2, node1)) {
        return ;
    }
    data.set(value, node1, node2);
#else
    if (data[node1][node2] || data[node2][node1]) {
        return ;
    }
    data[node1][node2] = value;
#endif
}

template <typename T>
inline void Matrix<T>::connect(T value, uint node1, uint node2) {
#ifdef SPARSE
    data.set(value, node1, node2);
    data.set(value, node2, node1);
#else
    data[node1][node2] = value;
    data[node2][node1] = value;
#endif       
}

template <typename T>
inline bool Matrix<T>::isConnected(uint node1, uint node2) const {
#ifdef SPARSE
    return data.get(node1, node2) && data.get(node2, node1);
#else
    return data[node1][node2] && data[node2][node1];
#endif
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
