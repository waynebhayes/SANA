#include "xrand.hpp"

using namespace std;

uint xrand(uint begin, uint end){
	if(begin < end){
		random_device rand_dev;
		mt19937 generator(rand_dev());
		uniform_int_distribution<uint>  distr(begin, end-1);
		return distr(generator);
	}
	else{
		throw out_of_range("xrand(begin, end): begin must be less than end");
	}
}