#include "xrand.hpp"

using namespace std;

ullint xrand(ullint begin, ullint end){
    if(begin < end){
        static random_device rand_dev;
        mt19937 generator(rand_dev());
        uniform_int_distribution<ullint>  distr(begin, end-1);
        return distr(generator);
    }
    else{
        throw out_of_range("xrand(begin, end): begin must be less than end");
    }
}
void xshuffle(vector<ullint>& nodes, ullint len){
    if(len > 0 and nodes.size() > 1){
           for(ullint i = 0; i < min(len, (ullint) nodes.size()); i++){
               ullint j = xrand(i, nodes.size());
               swap(nodes[i], nodes[j]);
           }
    }
}
