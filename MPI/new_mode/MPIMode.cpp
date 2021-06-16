
#include "MPIMode.hpp"
#include <utility>
#include <iostream>
#include "../utils/utils.hpp"
#include "../utils/FileIO.hpp"
#include "../arguments/measureSelector.hpp"
#include "../arguments/MethodSelector.hpp"
#include "../arguments/GraphLoader.hpp"
#include "../Report.hpp"

#include <mpi.h>
#include "../arguments/GraphLoader.hpp"

#include "../arguments/modeSelector.hpp"
#include "../modes/CreateShadow.hpp"
#include "../modes/NormalMode.hpp"
void MPIMode::run(ArgumentParser& args) {
    //MPI_Init(&argc, &argv);
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    cout << "mpi finish init" << endl;
    // create shadow
    cout << "creating shadow" << endl;
    args.strings["-fshadow"] = "MPI/shadowconf.txt";
    CreateShadow* s = new CreateShadow();
    s->run(args);
    delete s;

    //test----
    Graph shadow = GraphLoader::loadGraphFromFile("shadow", "MPI/shadownet.gw", true);
    cout << shadow.numColors() << endl;
    //--------

    // graphs from shadowconfig
    vector<string> graphFiles{"regression-tests/shadow/syeast10.el", "regression-tests/shadow/syeast15.el", "regression-tests/shadow/syeast20.el", "regression-tests/shadow/syeast25.el"};
    // initial alignment
    cout << "creating initial random alignment" << endl;
    args.strings["-mode"] = "dbg";
    args.strings["-method"] = "random";
    args.strings["-fg1"] = graphFiles[rank];
    args.strings["-fg2"] = "MPI/shadownet.gw";

    //args.strings["-startalignment"] = "a";
    string graphBasename = graphFiles[rank].substr(graphFiles[rank].find_last_of("/") + 1);
    graphBasename = graphBasename.substr(0, graphBasename.find_last_of("."));
    args.strings["-o"] = "MPI/" + graphBasename + "-init.align";
    args.writeArguments();
    cout << toLowerCase(args.strings["-method"]) << endl;
    Mode* normal = modeSelector::selectMode(args);
    normal->run(args);

    //args.strings["-fg1"] = graphFiles[rank];
    //args.strings["-fg2"] = "MPI/shadownet.gw";

}

string MPIMode::getName() { return "MPIMode"; }