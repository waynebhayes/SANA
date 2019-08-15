#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

template<class T>
void writeMatrixToFile(const vector<vector<T> >& matrix, const string& fileName) {
    uint n1 = matrix.size();
    uint n2 = matrix[0].size();
    ofstream fout(fileName.c_str());
    for (uint i = 0; i < n1; i++) {
        for (uint j = 0; j < n2; j++) {
            fout << matrix[i][j] << " ";
        }
        fout << endl;
    }
    fout.close();
}

template<class T>
void readMatrixFromFile(vector<vector<T> >& matrix, uint nrows, uint ncols, const string& fileName) {
    matrix = vector<vector<T> > (nrows, vector<T> (ncols));
    ifstream fin(fileName.c_str());
    for (uint i = 0; i < nrows; i++) {
        for (uint j = 0; j < ncols; j++) {
            fin >> matrix[i][j];
        }
    }
}

template<class T>
void writeMatrixToBinaryFile(const vector<vector<T> >& matrix, const string& fileName) {
    uint n1 = matrix.size();
    uint n2 = matrix[0].size();
    ofstream fout(fileName.c_str(), ios::out | ios::binary);
    for (uint i = 0; i < n1; i++) {
        fout.write((char*)&matrix[i][0], n2*sizeof(T));
    }
    fout.close();
}

template<class T>
void readMatrixFromBinaryFile(vector<vector<T> >& matrix, const string& fileName) {
    uint n1 = matrix.size();
    uint n2 = matrix[0].size();
    ifstream fin(fileName.c_str(), ios::in | ios::binary);
    for (uint i = 0; i < n1; i++) {
        fin.read((char*)&matrix[i][0], n2*sizeof(T));
    }
    fin.close();
}

template<class T>
T matrixMax(vector<vector<T> >& matrix) {
    uint n1 = matrix.size();
    uint n2 = matrix[0].size();
    T maxim = matrix[0][0];
    for (uint i = 0; i < n1; i++) {
        for (uint j = 0; j < n2; j++) {
            if (matrix[i][j] > maxim) maxim = matrix[i][j];
        }
    }
    return maxim;
}

template<class T>
T matrixMin(vector<vector<T> >& matrix) {
    uint n1 = matrix.size();
    uint n2 = matrix[0].size();
    T minim = matrix[0][0];
    for (uint i = 0; i < n1; i++) {
        for (uint j = 0; j < n2; j++) {
            if (matrix[i][j] < minim) minim = matrix[i][j];
        }
    }
    return minim;
}

template<typename T>
bool contains(const vector<T>& v, const T& elem) {
    return find(v.begin(), v.end(), elem) != v.end();
}

template <typename T>
string toStringWithPrecision(const T a_value, const int n = 6)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}