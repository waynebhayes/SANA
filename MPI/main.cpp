#include <iostream>
#include <mpi.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <fstream>

namespace patch{
    template < typename T > std::string to_string( const T& n ){
        std::ostringstream stm;
        stm << n;
        return stm.str();
    }
}

void print(std::string s){  // easier printing
    std::cout << s << std::endl;
}

// global variables
int num_iters = 5;
int time_per_iter = 1;
std::string measures = "-ms3 1 -ms3_type 1";
std::string name = "AAAmpi-shadow"; // outdir
std::string networksList[4] = {"regression-tests/shadow/syeast10.el", "regression-tests/shadow/syeast15.el", "regression-tests/shadow/syeast20.el", "regression-tests/shadow/syeast25.el"};


// iter, shadowFile, array of networks, size of network array, array of alignment files, size of alignment array
void createShadowConfig(std::string i, std::string shadowFile, std::string networks[], int size, std::string alignFiles[], int size2){
    std::ofstream file;
    system(("mkdir -p "+name+"/"+i).c_str());
    file.open((name + "/" + i + "/fshadow.txt").c_str());
    std::string data = "# file where the created shadow is stored\n" + shadowFile + "\n\n# number k of graphs to create the shadow, followed by the k graphs in any supported format\n" + patch::to_string(size) + "\n";
	for(int i = 0; i < size; i++){
        data += networks[i] + "\n";
    }
	if(size2 == 0){
        data += "\n# 0/1 indicating if alignment files are provided, followed by k files if so\n0\n";
    }else{
        data += "\n# 0/1 indicating if alignment files are provided, followed by k files if so\n1\n";
        for(int i = 0; i < size; i++){
            data += alignFiles[i] + "\n";
        }
    }
	data += "\n# 0/1 indicating if color files are provided\n0\n\n";
    data += "# number C of colors, followed by the C color names and their number of dummy nodes\n";
    data += "# (if not providing color files, the number of colors is 1 and the number of dummy nodes\n";
    data += "# must be specified for __default)\n";
    data += "1\n__default 110";
    file << data;
    file.close();
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv); // Initial

    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // USAGE: ./multi-pairwise.sh [-frugal] [-archive TYPE] [-H N] [-v|V] 
    // {multi-SANA.exe} 'measures' iterations time-per-iter parallel-spec outdir 
    // {list of input networks}
    
    // setup
    if(rank == 0){
        // CREATE INITIAL ALIGNMENT
        print("-CREATING INITIAL ALIGNMENT");
        system(("mkdir -p "+name+"/.init").c_str());
        system(("mkdir -p "+name+"/0").c_str());

        // random multi-alignment
        print("--Creating initial random multi-alignment");
        std::string networkFiles = "";
        for(int i=0; i<4; i++){
            networkFiles += networksList[i] + " ";
        }
        system(("./random-align.sh " + name + " " + networkFiles).c_str());

        // creating shadow config
        print("--Creating initial shadowConfig");
        std::string alignList[4];
        for(int i = 0; i < size; i++){
            std::string graph = networksList[i];
            std::string graphBasename = graph.substr(graph.find_last_of("/") + 1);
            graphBasename = graphBasename.substr(0, graphBasename.find_last_of("."));
            alignList[i] = name + "/.init/" + graphBasename + "-shadow.align";
        }
        std::string shadowGraph = name + "/0/" + name + "-shadow0.gw";
        std::string shadowConfig = name+"/0/fshadow.txt";
        createShadowConfig("0", shadowGraph, networksList, 4, alignList, 4);

        // initial shadow-graph
        print("--Creating initial shadow-graph");
        system(("./sana.multi -it 1 -mode shadow -fshadow "+ shadowConfig+ " > "+name+"/0/fshadow.stdout 2>"+name+"/0/fshadow.stderr").c_str());
        system(("mv " + name + "/.init/*-shadow.align " + name + "/0").c_str());

        print("Starting iterations \n");
    }

    // synchro
    MPI_Barrier(MPI_COMM_WORLD);
    
    // TODO: remove barrier syncs between loops. Update shadownetwork asynchronously
    // extra thread with "real" shadow network. Every iter, update changes, get a copy of the real one.
    for(int i = 0; i < 5; i++){ //outer loop
        
        print("ITER " + patch::to_string(i) + " on graph: " + networksList[rank]);
        int next_i = i + 1;
        double minutes = 0.1;

        std::string graph = networksList[rank];
        std::string graphBasename = graph.substr(graph.find_last_of("/") + 1);
        graphBasename = graphBasename.substr(0, graphBasename.find_last_of("."));

        system(("mkdir -p "+name+"/"+patch::to_string(next_i)).c_str());
        std::string shadowGraph = name + "/" + patch::to_string(i) + "/" + name + "-shadow" + patch::to_string(i) + ".gw";


        int sa_iters = 1; // how to calculate??? time_per_iter/0.015 ?
        system(("./sana.multi " + measures + " -multi-iteration-only -it "+ patch::to_string(sa_iters) +" -fg1 " + graph + " -fg2 " + shadowGraph + " -o " + name + "/" + patch::to_string(next_i) + "/" + graphBasename +"-shadow >"+ name +"/" + patch::to_string(next_i) + "/"+graphBasename+"-shadow.stdout 2>" + name +"/" + patch::to_string(next_i) + "/"+graphBasename+"-shadow.stderr -startalignment "+ name + "/" + patch::to_string(i) + "/"+graphBasename+"-shadow.align").c_str());

        MPI_Barrier(MPI_COMM_WORLD); // temporary barrier sync?
        if(rank == 0){
            //create shadowconfig for next outer loop
            shadowGraph = name + "/" + patch::to_string(next_i) +"/" + name + "-shadow" + patch::to_string(next_i) +".gw";
            std::string alignList[4];
            for(int i = 0; i < size; i++){
                std::string graph = networksList[i];
                std::string graphBasename = graph.substr(graph.find_last_of("/") + 1);
                graphBasename = graphBasename.substr(0, graphBasename.find_last_of("."));
                alignList[i] = name + "/" + patch::to_string(next_i) + "/" + graphBasename + "-shadow.align";
            }
            createShadowConfig(patch::to_string(next_i), shadowGraph, networksList, 4, alignList, 4);
            std::string shadowConfig = name + "/" + patch::to_string(next_i) + "/fshadow.txt";
            // creating fixed shadownetwork for next outer loop, syncing??
            print("creating fixed shadownetwork for next outer loop");
            system(("./sana.multi -it 1 -mode shadow -fshadow "+ shadowConfig+ " > "+name+"/" + patch::to_string(next_i) + "/fshadow.stdout 2>"+name+"/" + patch::to_string(next_i) + "/fshadow.stderr").c_str());
        }
        MPI_Barrier(MPI_COMM_WORLD); // need shadowfile before continuing
    }
    
    //system(("/bin/rm core.*").c_str());
    MPI_Finalize(); // Finalisation
}