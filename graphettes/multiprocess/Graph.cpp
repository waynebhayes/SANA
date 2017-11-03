#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <set>
#include <cmath>
#include <fstream>
#include <time.h>
#include <cassert>
#include "Graph.hpp"
using namespace std;

uint Graph::getNumNodes() const {
    return adjLists.size();
}

void Graph::getAdjLists(vector<vector<ushort> >& adjListsCopy) const {
    uint n = getNumNodes();
    adjListsCopy = vector<vector<ushort> > (n, vector<ushort> (0));
    for (uint i = 0; i < n; i++) {
        adjListsCopy[i] = adjLists[i];
    }
}

//note: does not update CCs
void Graph::addEdge(ushort node1, ushort node2) 
{
    adjMatrix[node1][node2] = adjMatrix[node2][node1] = true;
    adjLists[node1].push_back(node2);
    adjLists[node2].push_back(node1);
}


//My functions and constructor 
Graph::Graph(int n) :
    adjMatrix(vector<vector<bool>> (n, vector<bool>(n))),
    adjLists(vector<vector<ushort>> (n, vector<ushort>(0)))
    {}

void Graph::setAdjMatrix(vector<bool>& v)
{
    int k = 0;
    for (unsigned int i = 0; i < this->getNumNodes(); i++)
    {
        for (unsigned int j = 0; j < this->getNumNodes(); j++)
        {
            if (i < j)
            {
                adjMatrix[i][j] = v[k];
                if (v[k])
                {
                    this->addEdge(i,j);
                }
                k++;
            }
        }    
    }        
}

void Graph::print_adjMatrix(bool upper)
{
    for (unsigned int i = 0; i < this->getNumNodes(); i++)
    {
        for (unsigned int j = 0; j < this->getNumNodes(); j++)
        {
            if (upper)
            {
                if (i < j)
                    std::cout << adjMatrix[i][j] << " ";
                else
                    std::cout << "  ";
            }
            else
                std::cout << adjMatrix[i][j] << " ";
        }
        std::cout << "\n";
    }
}

uint Graph::getDegree(uint node) const
{
    return adjLists[node].size();
}

void Graph::set_decimal_representation(int n)
{
    decimal_representation = n;
}

int Graph::get_decimal_representation() const
{
    return decimal_representation;
}

void Graph::construct_decimal_representation()
{
    int result = 0;
    int k = 0;

    for (int i = adjMatrix.size() - 1; i >= 0; i--)
    {   
        for (int j = adjMatrix.size() - 1; j >= 0; j--)   
        {
            if (i < j)
            {
                if (adjMatrix[i][j])
                {
                    int n = pow(2, k);
                    result += n;
                }
                k++;
            }
        }
    }
    
    decimal_representation = result;
}

void Graph::print_adjLists()
{
    for (unsigned int i = 0; i < adjLists.size(); i++)
    {
        std::cout << "Start node: " << i << " --> ";
        for (unsigned int j = 0; j < adjLists[i].size(); j++)
        {
            std::cout << adjLists[i][j] << " ";
        }
        std::cout << "\n";
    }

}

void Graph::reset()
{
    // vector<vector<bool>> n(adjMatrix.size(), vector<bool>(adjMatrix.size()));
    // adjMatrix = n;
    for (vector<bool> m:adjMatrix) {
        for (uint i = 0; i < m.size(); i++) {
            m[i] = false;
        }
    }
    // vector<vector<ushort>> item(adjLists.size(), vector<ushort>(0));
    // adjLists = item;
    for (uint i = 0; i < adjLists.size(); i++) {
        adjLists[i] = vector<ushort>(0);
    }
}    