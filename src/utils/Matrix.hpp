#ifndef MATRIX_HPP_
#define MATRIX_HPP_

#include "utils.hpp"
#include "SparseMatrix.hpp"
#include <vector>
#include <algorithm>

using namespace std;

#ifdef SPARSE
    #define INNER_CONTAINER unordered_map<uint, T>
#endif

template <typename T>
class Matrix {
public:
    Matrix();
    Matrix(const Matrix & matrix);
    Matrix(Matrix && matrix) noexcept;
    Matrix(uint numberOfNodes);
    Matrix(uint row, uint col);
    ~Matrix();

    Matrix & operator = (const Matrix & matrix);
    Matrix & operator = (Matrix && matrix) noexcept;

#ifdef SPARSE
    INNER_CONTAINER & operator [] (uint node1);
#else
    // Row proxy for flat array - provides matrix[row][col] syntax
    struct RowProxy {
        T* ptr;
        uint cols;
        inline T& operator[](uint col) { return ptr[col]; }
        inline const T& operator[](uint col) const { return ptr[col]; }
        uint size() const { return cols; }
    };
    inline RowProxy operator [] (uint node1) {
        return RowProxy{flatData + (size_t)node1 * numCols, numCols};
    }
#endif

    inline const T get(uint node1, uint node2) const;
    uint size() const;

private:
#ifdef SPARSE
    SparseMatrix<T> data;
#else
    T* flatData;
    uint numRows;
    uint numCols;
#endif
};

// ============================================================
// SPARSE implementations (unchanged from original)
// ============================================================
#ifdef SPARSE

template <typename T>
inline INNER_CONTAINER & Matrix<T>::operator [] (uint node1) {
    return data[node1];
}

template <typename T>
Matrix<T>::Matrix() : data() {}

template <typename T>
Matrix<T>::Matrix(const Matrix & matrix) : data(matrix.data) {}

template <typename T>
Matrix<T>::Matrix(Matrix && matrix) noexcept : data(std::move(matrix.data)) {}

template <typename T>
Matrix<T>::Matrix(uint numberOfNodes) : data(numberOfNodes) {}

template <typename T>
Matrix<T>::Matrix(uint row, uint col) : data(row) {}

template <typename T>
Matrix<T>::~Matrix() {}

template <typename T>
Matrix<T> & Matrix<T>::operator = (const Matrix & matrix) {
    data = matrix.data;
    return *this;
}

template <typename T>
Matrix<T> & Matrix<T>::operator = (Matrix && matrix) noexcept {
    data = std::move(matrix.data);
    return *this;
}

template <typename T>
inline const T Matrix<T>::get(uint node1, uint node2) const {
    return data.get(node1, node2);
}

template <typename T>
uint Matrix<T>::size() const {
    return data.size();
}

// ============================================================
// Non-SPARSE: flat contiguous array
// Eliminates doubly-indirect heap layout (vector<vector<T>>)
// for much better cache locality and fewer pointer chases.
// ============================================================
#else

template <typename T>
Matrix<T>::Matrix() : flatData(nullptr), numRows(0), numCols(0) {}

template <typename T>
Matrix<T>::Matrix(const Matrix & matrix) : numRows(matrix.numRows), numCols(matrix.numCols) {
    size_t sz = (size_t)numRows * numCols;
    if (sz > 0) {
        flatData = new T[sz]();
        std::copy(matrix.flatData, matrix.flatData + sz, flatData);
    } else {
        flatData = nullptr;
    }
}

template <typename T>
Matrix<T>::Matrix(Matrix && matrix) noexcept
    : flatData(matrix.flatData), numRows(matrix.numRows), numCols(matrix.numCols) {
    matrix.flatData = nullptr;
    matrix.numRows = matrix.numCols = 0;
}

template <typename T>
Matrix<T>::Matrix(uint numberOfNodes) : numRows(numberOfNodes), numCols(numberOfNodes) {
    size_t sz = (size_t)numberOfNodes * numberOfNodes;
    flatData = sz > 0 ? new T[sz]() : nullptr;
}

template <typename T>
Matrix<T>::Matrix(uint row, uint col) : numRows(row), numCols(col) {
    size_t sz = (size_t)row * col;
    flatData = sz > 0 ? new T[sz]() : nullptr;
}

template <typename T>
Matrix<T>::~Matrix() {
    delete[] flatData;
}

template <typename T>
Matrix<T> & Matrix<T>::operator = (const Matrix & matrix) {
    if (this != &matrix) {
        delete[] flatData;
        numRows = matrix.numRows;
        numCols = matrix.numCols;
        size_t sz = (size_t)numRows * numCols;
        if (sz > 0) {
            flatData = new T[sz];
            std::copy(matrix.flatData, matrix.flatData + sz, flatData);
        } else {
            flatData = nullptr;
        }
    }
    return *this;
}

template <typename T>
Matrix<T> & Matrix<T>::operator = (Matrix && matrix) noexcept {
    if (this != &matrix) {
        delete[] flatData;
        flatData = matrix.flatData;
        numRows = matrix.numRows;
        numCols = matrix.numCols;
        matrix.flatData = nullptr;
        matrix.numRows = matrix.numCols = 0;
    }
    return *this;
}

template <typename T>
inline const T Matrix<T>::get(uint node1, uint node2) const {
    return flatData[(size_t)node1 * numCols + node2];
}

template <typename T>
uint Matrix<T>::size() const {
    return numRows;
}

#endif // SPARSE

#endif /* MATRIX_HPP_ */
