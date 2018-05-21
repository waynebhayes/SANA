#include <vector>
#include <iostream>

#include "SANA.hpp"
#include "Graph.hpp"
#include "SANAResult.hpp"
#include "PairwiseNetPile.hpp"
#include "TemperatureSchedule.hpp"

SANA::SANA() {

}

SANA::SANA(const SANAConfiguration &sanaConfig): config(sanaConfig) {

}

SANAResult SANA::StartAligner(const vector<Graph> &networkList) {
    std::cout << networkList.size() << std::endl; // TOOD, just dummy print for now
}
