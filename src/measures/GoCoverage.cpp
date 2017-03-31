#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include "GoCoverage.hpp"
#include "localMeasures/GoSimilarity.hpp"

using namespace std;

GoCoverage::GoCoverage(Graph* G1, Graph* G2) :
    Measure(G1, G2, "gocov") {
    NORMALIZE = true;
}

GoCoverage::~GoCoverage() {

}

double GoCoverage::permutationInverse(uint M, uint N) {
    double res = 1;
    uint aux = M;
    while (aux > (M-N)) {
        res /= aux;
        aux--;
    }
    return res;
}

double GoCoverage::downweightedScore(uint M, uint N) {
    return 1.0/N * permutationInverse(M, N);
}

double GoCoverage::scoreUpperBound() {
    unordered_map<uint,uint> goCountG1 = GoSimilarity::getGoCounts(*G1);
    unordered_map<uint,uint> goCountG2 = GoSimilarity::getGoCounts(*G2);
    double total = 0;
    for (auto pair : goCountG1) {
        uint goTerm = pair.first;
        uint M = pair.second;
        uint N = goCountG2[goTerm];
        if (N > 0) {
            if (N > M) swap(N, M);
            total += permutationInverse(M, N);
        }
    }
    return total;
}

double GoCoverage::eval(const Alignment& A) {
    uint n1 = G1->getNumNodes();

    vector<vector<uint> > goTermsG1 = GoSimilarity::loadGOTerms(*G1, 1);
    vector<vector<uint> > goTermsG2 = GoSimilarity::loadGOTerms(*G2, 1);
    unordered_map<uint,uint> goCountG1 = GoSimilarity::getGoCounts(*G1);
    unordered_map<uint,uint> goCountG2 = GoSimilarity::getGoCounts(*G2);

    map<ushort,string> namesG1 = G1->getIndexToNodeNameMap();
    map<ushort,string> namesG2 = G2->getIndexToNodeNameMap();

    double total = 0;
    for (uint i = 0; i < n1; i++) {
        for (uint goTerm : goTermsG1[i]) {
            if (contains(goTermsG2[A[i]], goTerm)) {
                uint M = goCountG1[goTerm];
                uint N = goCountG2[goTerm];
                if (N > M) swap(N, M);
                total += downweightedScore(M, N);
            }
        }
    }
    if (NORMALIZE) return total/scoreUpperBound();
    else return total;
}

