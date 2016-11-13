#include "HalfMatrix.hpp"

using namespace std;
HalfMatrix::HalfMatrix(uint n, std::vector<bool>& bitVector){
    if (n == 0)
        assert("Error: HalfMatrix constructor has 0 size");
    len_ = n;
    _bitArray = new bool[(n*n-n)/2];
    for(uint i = 0; i < bitVector.size(); i++){
        _bitArray[i] = bitVector[i];
    }
}

HalfMatrix::HalfMatrix(uint n, uint decimalNumber){
    if (n == 0)
        assert("Error: HalfMatrix constructor has 0 size");
    len_ = n;
    _bitArray = new bool[(n*n-n)/2];
    this->encodeBitArray(decimalNumber);
}

HalfMatrix::HalfMatrix(uint n)
                : HalfMatrix(n, 0){

}

HalfMatrix::HalfMatrix(const HalfMatrix& m){
    len_ = m.len_;
    uint size = (len_*len_-len_)/2;
    _bitArray = new bool[size];
    for( uint i = 0; i < size; i++)
        _bitArray[i] = m._bitArray[i];
}

void HalfMatrix::encodeBitArray(uint decimalNumber){
    // Convert to binary number and put each bit in result vector.
    uint num_edges = (len_*len_-len_)/2;
    for(uint k = 0; k < num_edges; k++) 
        _bitArray[k] = false;
    
    uint i = (len_*len_ - len_)/2 - 1;
    while (decimalNumber != 0)
    {
        assert(i >= 0 && "Binary length exceeds number of possible edges");
        _bitArray[i] = decimalNumber % 2;
        --i;
        decimalNumber /= 2; 
    }
}

HalfMatrix::~HalfMatrix(){
    delete[] _bitArray;
}

bool& HalfMatrix::operator() (uint row, uint col){
    if (row >= len_ or col >= len_)
        throw std::out_of_range("HalfMatrix: index out of range");
    uint pos;
    if(row < col)
        pos = row*len_ + col-((row+1)*(row+2))/2;
    else if(row > col)
        pos = col*len_ + row-((col+1)*(col+2))/2;
    else
        throw std::out_of_range("bad HalfMatrix index: row and column can't be equal");
    return _bitArray[pos];
}

HalfMatrix& HalfMatrix::operator= (const HalfMatrix& m){
    len_ = m.len_;
    uint size = (len_*len_-len_)/2;
    if(_bitArray != NULL)
        delete[] _bitArray;
    //I don't want the new _bitArray to be modified when the m._bitArray
    // gets modified by some supernatural entity. Hence no pointer copying.
    _bitArray = new bool[size];
    for( uint i = 0; i < size; i++)
        _bitArray[i] = m._bitArray[i];
    return *this;
}

void HalfMatrix::clear(){
    if(_bitArray != NULL)
        delete[] _bitArray;
    _bitArray = NULL;
}