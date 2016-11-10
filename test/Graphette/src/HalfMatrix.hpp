#ifndef HALFMATRIX_HPP
#define HALFMATRIX_HPP

#include <vector>
#include <cstdlib>
#include <cassert>
#include <stdexcept>
#include <iostream>

typedef unsigned int uint;
typedef unsigned short int ushort;

class HalfMatrix{
public:
    HalfMatrix(){};
    HalfMatrix(ushort n, std::vector<bool>& bitVector);
    HalfMatrix(ushort n, uint decimalNumber);
    HalfMatrix(ushort n);
    HalfMatrix(const HalfMatrix& m);   // Copy constructor
    ~HalfMatrix();

    bool& operator() (ushort row, ushort col);
    //bool operator() (ushort row, ushort col) const;
    HalfMatrix& operator= (const HalfMatrix& m);
    void clear();
private:
    ushort len_;
   	bool* _bitArray = NULL;
   	void encodeBitArray(uint decimalNumber);
};
#endif