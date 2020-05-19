#include "HalfMatrix.hpp"
using namespace std;

HalfMatrix::HalfMatrix(ullint n, vector<bool>& bitVector){
    if (n == 0)
        throw invalid_argument("HalfMatrix(n, bitVector): n can't be 0");
    else if ((n*n-n)/2 != bitVector.size())
        throw invalid_argument("HalfMatrix(n, bitVector): bitVector.size() is not (n*n-n)/2");
    else{
        len_ = n;
        bitArray_ = new bool[(n*n-n)/2];
        for(ullint i = 0; i < bitVector.size(); i++){
            bitArray_[i] = bitVector[i];
        }
    }
}

HalfMatrix::HalfMatrix(ullint n, ullint decimalNumber){
    if (n == 0)
        throw invalid_argument("HalfMatrix constructor has size = 0");
    len_ = n;
    bitArray_ = new bool[(n*n-n)/2];
    this->encodeBitArray(decimalNumber);
}

HalfMatrix::HalfMatrix(ullint n)
                : HalfMatrix(n, 0){

}

HalfMatrix::HalfMatrix(const HalfMatrix& m){
    len_ = m.len_;
    ullint size = (len_*len_-len_)/2;
    bitArray_ = new bool[size];
    for( ullint i = 0; i < size; i++)
        bitArray_[i] = m.bitArray_[i];
}

void HalfMatrix::encodeBitArray(ullint decimalNumber){
    // Convert to binary number and put each bit in result vector.
    ullint num_edges = (len_*len_-len_)/2;
    for(ullint k = 0; k < num_edges; k++) 
        bitArray_[k] = false;
    
    int i = (len_*len_ - len_)/2 - 1;
    while (decimalNumber != 0)
    {
        if (i < 0){
            throw out_of_range("HalfMatrix: Binary length exceeds number of possible edges");
        }
        else{
            bitArray_[i] = decimalNumber % 2;
            --i;
            decimalNumber /= 2;
        } 
    }
}

HalfMatrix::~HalfMatrix(){
    delete[] bitArray_;
}

ullint HalfMatrix::length(){
    return len_;
}

void HalfMatrix::print(){
    for (ullint i = 0; i < len_; i++){
        for (ullint j = 0; j < len_; j++){
            if (i < j)
                cout << (*this)(i, j) << " ";
            else
                cout << "  ";
        }
        cout << "\n";
    }
}

bool& HalfMatrix::operator() (ullint row, ullint col){
    if (row >= len_ or col >= len_)
        throw out_of_range("HalfMatrix: index out of range");
    ullint pos;
    if(row < col)
        pos = row*len_ + col-((row+1)*(row+2))/2;
    else if(row > col)
        pos = col*len_ + row-((col+1)*(col+2))/2;
    else
        throw out_of_range("HalfMatrix: row and col equal");
    return bitArray_[pos];
}

HalfMatrix& HalfMatrix::operator= (const HalfMatrix& m){
    len_ = m.len_;
    ullint size = (len_*len_-len_)/2;
    if(bitArray_ != NULL)
        delete[] bitArray_;
    //I don't want the new bitArray_ to be modified when the m.bitArray_
    // gets modified by some supernatural entity. Hence no pointer copying.
    bitArray_ = new bool[size];
    for( ullint i = 0; i < size; i++)
        bitArray_[i] = m.bitArray_[i];
    return *this;
}

void HalfMatrix::clear(){
    if(bitArray_ != NULL)
        delete[] bitArray_;
    bitArray_ = NULL;
}