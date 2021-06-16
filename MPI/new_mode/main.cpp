#include "../../src/arguments/ArgumentParser.hpp"
#include "../../src/arguments/modeSelector.hpp"
#include "../../src/utils/randomSeed.hpp"
#include "../../src/utils/utils.hpp"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

using namespace std;

int main(int argc, char* argv[]) {
#define MPI 1
    cout << unitbuf; //set cout to flush after each insertion

    ArgumentParser args(argc, argv);
#if MPI
    //mpi init if mode is mpi
    if(args.strings["-mode"] == "mpi"){
        MPI_Init(&argc, &argv);
    }
#endif //MPI

    // Assign to unused_ret to shut the compiler warning about igoring return value
    int sysRet = system("hostname -f; date");
    if (sysRet != 0) cerr<<"'hostname -f; date' returned error code "<<sysRet<<endl;

    args.writeArguments();
    
    if(args.doubles["-seed"] != 0) setSeed(args.doubles["-seed"]);
    else setRandomSeed();
    cout<<"Seed: "<<getRandomSeed()<<endl;

    Mode* mode = modeSelector::selectMode(args);
    mode->run(args);
    delete mode;
#if MPI
    if(args.strings["-mode"] == "mpi"){
        MPI_Finalize(); // Finalisation
    }
#endif //MPI
}
