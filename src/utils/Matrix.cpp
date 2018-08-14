#include "Matrix.hpp"

Matrix::Matrix() {
}

Matrix::Matrix(const Matrix & matrix) {
    data = matrix.data;
}

Matrix::Matrix(uint numberOfNodes) {
#ifdef SPARSE
    data = MATRIX_DATA_STRUCTURE(numberOfNodes);
#else
    data = MATRIX_DATA_STRUCTURE(numberOfNodes, 
           vector<VALUE_UNIT>(numberOfNodes, VALUE_UNIT()));
#endif        
}

Matrix & Matrix::operator = (const Matrix & matrix) {
    data = matrix.data;
    return *this;
}

uint Matrix::size() const {
    return data.size();
}
