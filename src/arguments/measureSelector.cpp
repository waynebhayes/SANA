#include <cassert>
#include <iostream>

#include "measureSelector.hpp"

#include "../measures/EdgeCorrectness.hpp"
#include "../measures/InducedConservedStructure.hpp"
#include "../measures/SymmetricSubstructureScore.hpp"
#include "../measures/LargestCommonConnectedSubgraph.hpp"
#include "../measures/GoAverage.hpp"
#include "../measures/GoCoverage.hpp"
#include "../measures/WeightedEdgeConservation.hpp"
#include "../measures/NodeCorrectness.hpp"

#include "../measures/localMeasures/NodeDensity.hpp"
#include "../measures/localMeasures/EdgeDensity.hpp"
#include "../measures/localMeasures/GoSimilarity.hpp"
#include "../measures/localMeasures/Importance.hpp"
#include "../measures/localMeasures/Sequence.hpp"
#include "../measures/localMeasures/Graphlet.hpp"
#include "../measures/localMeasures/GraphletLGraal.hpp"

using namespace std;

bool shouldInit(string name, double weight, bool detailedReport, double wecWeight, string wecNodeSim) {
    if (detailedReport or weight > 0) return true;
    return wecWeight > 0 and wecNodeSim == name;
}

void initMeasures(MeasureCombination& M, Graph& G1, Graph& G2, ArgumentParser& args) {
    cerr << "Initializing measures... " << endl;
    Timer T;
    T.start();
    Measure *m;

    //detailedReport; if false, init only basic measures and any measure necessary to run SANA
    bool detRep = args.bools["-detailedreport"];

    m = new EdgeCorrectness(&G1, &G2);
    M.addMeasure(m, args.doubles["-ec"]);

    m = new InducedConservedStructure(&G1, &G2);
    M.addMeasure(m);

    m = new SymmetricSubstructureScore(&G1, &G2);
    M.addMeasure(m, args.doubles["-s3"]);
    m = new LargestCommonConnectedSubgraph(&G1, &G2);
    M.addMeasure(m);

    //local measures must be initialized before wec,
    //as wec uses one of the local measures
    double wecWeight = args.doubles["-wec"];
    string wecNodeSim = args.strings["-wecnodesim"];

    double nodedWeight = args.doubles["-noded"];
    if (shouldInit("noded", nodedWeight, detRep, wecWeight, wecNodeSim)) {
        m = new NodeDensity(&G1, &G2, args.vectors["-nodedweights"]);
        M.addMeasure(m, nodedWeight);
    }

    double edgedWeight = args.doubles["-edged"];
    if (shouldInit("edged", edgedWeight, detRep, wecWeight, wecNodeSim)) {
        m = new EdgeDensity(&G1, &G2, args.vectors["-edgedweights"]);
        M.addMeasure(m, edgedWeight);
    }

    if (GoSimilarity::fulfillsPrereqs(&G1, &G2)) {
        double goWeight = args.doubles["-go"];
        if (shouldInit("go", goWeight, detRep, wecWeight, wecNodeSim)) {
            m = new GoSimilarity(&G1, &G2, args.vectors["-goweights"]);
            M.addMeasure(m, goWeight);
        }
        if (detRep) {
            m = new GoCoverage(&G1, &G2);
            M.addMeasure(m);
        }
        //commented because it takes really long to compute,
        //and only works with yeast and human networks
        // m = new GoAverage(&G1, &G2);
        // M.addMeasure(m);
    }

    if (Importance::fulfillsPrereqs(&G1, &G2)) {
        double impWeight = args.doubles["-importance"];
        if (shouldInit("importance", impWeight, detRep, wecWeight, wecNodeSim)) {
            m = new Importance(&G1, &G2);
            M.addMeasure(m, impWeight);
        }
    }

    string blastFile = "sequence/scores/"+G1.getName()+"_"+G2.getName()+"_blast.out";
    if (fileExists(blastFile)) {
        double seqWeight = args.doubles["-sequence"];
        if (shouldInit("sequence", seqWeight, detRep, wecWeight, wecNodeSim)) {
            m = new Sequence(&G1, &G2);
            M.addMeasure(m, seqWeight);
        }
    }

    double graphletWeight = args.doubles["-graphlet"];
    if (shouldInit("graphlet", graphletWeight, detRep, wecWeight, wecNodeSim)) {
        m = new Graphlet(&G1, &G2);
        M.addMeasure(m, graphletWeight);
    }
    double graphletLgraalWeight = args.doubles["-graphletlgraal"];
    if (shouldInit("graphletlgraal", graphletLgraalWeight, detRep, wecWeight, wecNodeSim)) {
        m = new GraphletLGraal(&G1, &G2);
        M.addMeasure(m, graphletLgraalWeight);
    }

    if (detRep or wecWeight > 0) {
        LocalMeasure* nodeSim = (LocalMeasure*) M.getMeasure(args.strings["-wecnodesim"]);
        m = new WeightedEdgeConservation(&G1, &G2, nodeSim);
        M.addMeasure(m, wecWeight);
    }

    //NC is evaluated iff G1 and G2 have the same size
    //(for the networks we have, only happens in the syeast dataset)
    //alternatively, an argument '-evalnc' could be used
    if (G1.getNumNodes() == G2.getNumNodes()) {
        if (args.strings["-truealignment"] == "") {
            m = new NodeCorrectness(Alignment::identity(G1.getNumNodes()));
        }
        else {
            m = new NodeCorrectness(Alignment::loadMapping(args.strings["-truealignment"]));
        }
        M.addMeasure(m);
    }

    //commented because it requires distance matrices,
    //which can take hours to compute:
    // m = new ShortestPathConservation(&G1, &G2);
    // M.addMeasure(m);

    M.normalize();
    cerr << "done (" << T.elapsedString() << ")" << endl;
}

