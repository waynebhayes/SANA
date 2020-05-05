#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

template <typename T>
string toStringWithPrecision(const T a_value, const int n = 6)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}