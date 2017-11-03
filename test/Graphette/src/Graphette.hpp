#ifndef GRAPHETTE_HPP
#define GRAPHETTE_HPP
#include "HalfMatrix.hpp"
#include <bits/stdc++.h>

class Graphette{
public:
    Graphette(); //default constructor
    Graphette(ullint n, ullint decimalNumber);
    Graphette(ullint n, std::vector<bool>& bitVector);
    Graphette(ullint n, HalfMatrix adjMatrix);
    ~Graphette();
    ullint decimalNumber();
    std::vector<bool> bitVector();
    ullint numNodes();
    ullint numEdges();
    ullint degree(ullint node);
    ullint label(ullint node);
    std::vector <ullint> labels();
    void setLabel(ullint node, ullint label);
    void setLabels(std::vector<ullint>& label);
    void printAdjMatrix();
    std::vector<Graphette*> static generateAll(ullint n);
    std::vector<std::vector<ullint>> orbits();

private:
    ullint numNodes_;
    ullint numEdges_, decimalNumber_;
    HalfMatrix adjMatrix_;
    std::vector<ullint> degree_, label_;
    ullint decodeHalfMatrix();
    void init();
    
    bool suitable(std::vector<ullint>& permutation);
    Graphette* permuteNodes(std::vector<ullint>& permutation);
    void captureCycles(std::vector<ullint>& permutation, std::vector<ullint>& orbit);
    void followTrail(std::vector<ullint>& permutation, std::vector<ullint>& cycle,
                        ullint seed, ullint current, std::vector<bool>& visited);
};
#endif