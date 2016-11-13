#ifndef HALFMATRIX_HPP
#define HALFMATRIX_HPP

#include <vector>
#include <cstdlib>
#include <stdexcept>
#include <iostream>

typedef unsigned int uint;

class HalfMatrix{
public:
    HalfMatrix(){};
    HalfMatrix(uint n, std::vector<bool>& bitVector);
    HalfMatrix(uint n, uint decimalNumber);
    HalfMatrix(uint n);
    HalfMatrix(const HalfMatrix& m);   // Copy constructor
    ~HalfMatrix();

    bool& operator() (uint row, uint col);
    //bool operator() (uint row, uint col) const;
    HalfMatrix& operator= (const HalfMatrix& m);
    void clear();
private:
    uint _len;
   	bool* _bitArray = NULL;
   	void encodeBitArray(uint decimalNumber);
};
#endif