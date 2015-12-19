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

void initMeasures(MeasureCombination& M, Graph& G1, Graph& G2, ArgumentParser& args) {
    cerr << "Initializing measures... " << endl;
    Timer T;
    T.start();
    Measure *m;

    //if not true, init only basic measures and any measure necessary to run SANA
    bool detRep = args.bools["-detailedreport"];

    m = new EdgeCorrectness(&G1, &G2);
    M.addMeasure(m, args.doubles["-ec"]);

    m = new InducedConservedStructure(&G1, &G2);
    M.addMeasure(m);

    m = new SymmetricSubstructureScore(&G1, &G2);
    M.addMeasure(m, args.doubles["-s3"]);
    m = new LargestCommonConnectedSubgraph(&G1, &G2);
    M.addMeasure(m);

    double wecWeight = args.doubles["-wec"];

    double nodedWeight = args.doubles["-noded"];
    if (detRep or nodedWeight > 0 or (wecWeight > 0 and args.strings["-wecnodesim"] == "noded")) {
        m = new NodeDensity(&G1, &G2, args.vectors["-nodedweights"]);
        M.addMeasure(m, nodedWeight);
    }

    double edgedWeight = args.doubles["-edged"];
    if (detRep or edgedWeight > 0 or (wecWeight > 0 and args.strings["-wecnodesim"] == "edged")) {
        m = new EdgeDensity(&G1, &G2, args.vectors["-edgedweights"]);
        M.addMeasure(m, edgedWeight);
    }

    if (GoSimilarity::fulfillsPrereqs(&G1, &G2)) {
        double goWeight = args.doubles["-go"];
        if (detRep or goWeight > 0 or (wecWeight > 0 and args.strings["-wecnodesim"] == "go")) {
            m = new GoSimilarity(&G1, &G2, args.vectors["-goweights"]);
            M.addMeasure(m, goWeight);
        }
        if (args.bools["-goavg"]) {
            m = new GoAverage(&G1, &G2);
            M.addMeasure(m);
        }
        if (detRep) {
            m = new GoCoverage(&G1, &G2);
            M.addMeasure(m);
        }
    }

    if (Importance::fulfillsPrereqs(&G1, &G2)) {
        double impWeight = args.doubles["-importance"];
        if (detRep or impWeight > 0 or (wecWeight > 0 and args.strings["-wecnodesim"] == "importance")) {
            m = new Importance(&G1, &G2);
            M.addMeasure(m, impWeight);
        }
    }

    string blastFile = "sequence/scores/"+G1.getName()+"_"+G2.getName()+"_blast.out";
    if (fileExists(blastFile)) {
        double seqWeight = args.doubles["-sequence"];
        if (detRep or seqWeight > 0 or (wecWeight > 0 and args.strings["-wecnodesim"] == "sequence")) {
            m = new Sequence(&G1, &G2);
            M.addMeasure(m, seqWeight);
        }
    }

    double graphletWeight = args.doubles["-graphlet"];
    if (detRep or graphletWeight > 0 or (wecWeight > 0 and args.strings["-wecnodesim"] == "graphlet")) {
        m = new Graphlet(&G1, &G2);
        M.addMeasure(m, graphletWeight);
    }
    double graphletLgraalWeight = args.doubles["-graphletlgraal"];
    if (detRep or graphletLgraalWeight > 0 or (wecWeight > 0 and args.strings["-wecnodesim"] == "graphletlgraal")) {
        m = new GraphletLGraal(&G1, &G2);
        M.addMeasure(m, graphletLgraalWeight);
    }

    if (detRep or wecWeight > 0) {
        LocalMeasure* wecNodeSim = (LocalMeasure*) M.getMeasure(args.strings["-wecnodesim"]);
        m = new WeightedEdgeConservation(&G1, &G2, wecNodeSim);
        M.addMeasure(m, wecWeight);
    }

    if (G1.getNumNodes() == G2.getNumNodes()) { //assume that we are aligning a network with itself
        if (args.strings["-truealignment"] == "") {
            m = new NodeCorrectness(Alignment::identity(G1.getNumNodes()));
        }
        else {
            m = new NodeCorrectness(Alignment::loadMapping(args.strings["-truealignment"]));
        }
        M.addMeasure(m);
    }

    //m = new ShortestPathConservation(&G1, &G2);
    //M.addMeasure(m);

    M.normalize();
    cerr << "done (" << T.elapsedString() << ")" << endl;
}

