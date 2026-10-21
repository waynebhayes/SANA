#include "FMeasure.hpp"
#include <cmath>
#include <stdexcept>
using namespace std;

FMeasure::FMeasure(const Graph* G1, const Graph* G2, const double input_beta)
    : Measure(G1, G2, "f_beta") {
    
    const double density_threshold = (3 - std::sqrt(2)) / 4;

    

    if (input_beta == -1) {  
        if (G1->getNumNodes() <= 1) 
            throw std::invalid_argument("Graph must have more than one node to calculate density.");
    
        double network_density = (G1->getNumEdges()) / (0.5 * G1->getNumNodes() * (G1->getNumNodes() - 1));

        if (network_density < density_threshold) {
            beta = 1 / (4 * (1 - network_density));
        } else {
            double discriminant = -7 + (network_density * 24) - (16 * network_density * network_density);
            if (discriminant < 0) {
                throw std::domain_error("Invalid network density for calculating beta.");
            }
            beta = (1 + std::sqrt(discriminant)) / (4 * (1 - network_density));
        }
    } else {
        beta = input_beta;
    }
}


FMeasure::~FMeasure(){}

double FMeasure::eval(const Alignment& A) {
    double alignedEdges = A.computeNumAlignedEdges(*G1, *G2);
    double inducedEdgesG2 = G2->numEdgesInNodeInducedSubgraph(A.asVector());
    
    if(beta==inf){
        if (inducedEdgesG2 == 0) {
            // If there are no induced edges in G2, return 0.0 to avoid division by zero
            return 0.0;
        }
        return alignedEdges / inducedEdgesG2;
    }
    
    double totalEdgesG1 = G1->getNumEdges();
    double numerator = (1 + beta * beta) * alignedEdges;
    double denominator = totalEdgesG1 + (beta * beta * inducedEdgesG2);


    if (denominator == 0) {
        return 0.0;
    }

    return numerator / denominator;
}
