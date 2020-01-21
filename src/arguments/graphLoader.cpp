#include <string>
#include <thread>
#include <chrono>
#include <iostream>
#include <set>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <thread>
#include "graphLoader.hpp"
#include "../utils/Timer.hpp"

using namespace std;
bool _graphsSwitched = false;
/*
The program requires that there exist the network files in GW format
in networks/g1name/g1name.gw and networks/g1name/g2name.gw.

The -g1 and -g2 arguments allow you to specify g1name and g2name directly.
These arguments assume that the files already exist.

The -fg1 and -fg2 arguments allow you to specify external files containing
the graph definitions (in either GW or edge list format). If these
arguments are used, -g1 and -g2 are ignored. g1Name and g2Name are deduced
from the file names (by removing the path and the extension). Then,
the network definitions are parsed and the necessary network files are created.

 */
 
void initGraphs(Graph& G1, Graph& G2, ArgumentParser& args) {
    cout << "Initializing graphs..." << endl;
    Timer T;
    T.start();
    string fg1 = args.strings["-fg1"], fg2 = args.strings["-fg2"], path1 = args.strings["-pathmap1"], path2 = args.strings["-pathmap2"];
    createFolder("networks");
    createFolder(Graph::autogenFilesFolder());
    string g1Name, g2Name;
    uint p1 = 1, p2 = 1;
	
    if (fg1 != "") {
        g1Name = extractFileNameNoExtension(fg1);
    } else {
        g1Name = args.strings["-g1"];
    }
    if (fg2 != "") {
        g2Name = extractFileNameNoExtension(fg2);
    } else {
        g2Name = args.strings["-g2"];
    }

    if (path1 != "") {
        p1 = atoi(path1.c_str());
    }
    if (path2 != "") {
        p2 = atoi(path2.c_str());
    }

    string g1Folder, g2Folder;
    g1Folder = "networks/"+g1Name;
    g2Folder = "networks/"+g2Name;


    string g1GWFile, g2GWFile;
    g1GWFile = g1Folder+"/"+g1Name+".gw";
    // if (fileExists(g1GWFile) and fg1 != "") {
    //     cout << "Warning: argument of -fg1 (" << fg1 <<
    //             ") ignored because there already exists a network named " << g1Name << endl;
    // }
    g2GWFile = g2Folder+"/"+g2Name+".gw";
    // if (fileExists(g2GWFile) and fg2 != "") {
    //     cout << "Warning: argument of -fg2 (" << fg2 <<
    //             ") ignored because there already exists a network named " << g2Name << endl;
    // }

    if (not fileExists(g1GWFile)) {
        if (fg1 != "") {
            // if (fileExists(fg1)) {
            //     if (fg1.size() > 3 and fg1.substr(fg1.size()-3) == ".gw") {
            //         exec("cp "+fg1+" "+g1GWFile);
            //     } else {
            //         Timer tConvert;
            //         tConvert.start();
            //         cout << "Converting g1 to .gw (";
            //         Graph::edgeList2gw(fg1, g1GWFile);
            //         cout << tConvert.elapsedString() << "s)" << endl;
            //     }
            // } else {
            //     throw runtime_error("File not found: "+fg1);
            // }
        } else {
            throw runtime_error("File not found: " + g1GWFile);
        }
    }
    if (not fileExists(g2GWFile)) {
        if (fg2 != "") {
            // if (fileExists(fg2)) {
            //     if (fg2.size() > 3 and fg2.substr(fg2.size()-3) == ".gw") {
            //         exec("cp "+fg2+" "+g2GWFile);
            //     } else {
            //         Timer tConvert;
            //         tConvert.start();
            //         cout << "Converting g2 to .gw (";
            //         Graph::edgeList2gw(fg2, g2GWFile);
            //         cout << tConvert.elapsedString() << "s)" << endl;
            //     }
            // } else {
            //     throw runtime_error("File not found: "+fg2);
            // }
        } else {
            throw runtime_error("File not found: " + g2GWFile);
        }
    }

    // Reading the locked nodes
    vector<string> column1;
    vector<string> column2;

    string lockFile = args.strings["-lock"];
    if(lockFile != "")
    {
        if(fileExists(lockFile))
        {
            checkFileExists(lockFile);
            cout << "Locking the nodes in " << lockFile << endl;
            ifstream ifs(lockFile.c_str());
            string node;
            column1.reserve(14000);
            column2.reserve(14000);
            while(ifs >> node)
            {
                column1.push_back(node);
                ifs >> node;
                column2.push_back(node);
            }
            column1.shrink_to_fit();
            column2.shrink_to_fit();
        }
        else{
            cout << "Lock file (" << lockFile << ") does not exist!" << endl;
            throw runtime_error("Lock file not found: " + lockFile);
        }
    }
    
#if USE_CACHED_FILES
    bool multi_pairwise = false;
#if MULTI_PAIRWISE
    multi_pairwise = true;
#endif
    bool lockExists = false;
// By default, USE_CACHED_FILES is 0 and SANA does not cache files. Change USE_CACHED_FILES at your own risk.
    string g1LockDir = Graph::serializedFilesFolder() + g1Name + ".lock";
    string g2LockDir = Graph::serializedFilesFolder() + g2Name + ".lock";
    struct stat st;
    if (!stat(g1LockDir.c_str(), &st) || !stat(g2LockDir.c_str(), &st))
        lockExists = true;

    while (lockExists)
    {
        cout << "Waiting for serialization to complete...\n" << endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
        if (stat(g1LockDir.c_str(), &st) && stat(g2LockDir.c_str(), &st))
            lockExists = false;
    }
#endif

    bool updateG1 = true;
    bool updateG2 = true;
    bool usingLocks = args.bools["-lock-same-names"] || lockFile != "";

#if USE_CACHED_FILES
// By default, USE_CACHED_FILES is 0 and SANA does not cache files. Change USE_CACHED_FILES at your own risk.
    if (fg1 == "")
    {
        string gw = "networks/"+g1Name+"/"+g1Name+".gw";
        string bin = Graph::serializedFilesFolder() + g1Name + (args.bools["-bipartite"] ? "_bipartite" : "_UntypedNodes")
        + (usingLocks ? "_Locked" : "_Unlocked") + (multi_pairwise ? "_Multi_pairwise" : "_Not_multi_pairwise") + ".bin";
        updateG1 = newerGraphAvailable(gw.c_str(), bin.c_str());
    }
    else
    {
        string bin = Graph::serializedFilesFolder() + g1Name + (args.bools["-bipartite"] ? "_bipartite" : "_UntypedNodes") 
        + (usingLocks ? "_Locked" : "_Unlocked") + (multi_pairwise ? "_Multi_pairwise" : "_Not_multi_pairwise") + ".bin";
        updateG1 = newerGraphAvailable(fg1.c_str(), bin.c_str());
    }
	
    if (fg2 == "")
    {
        string gw = "networks/"+g2Name+"/"+g2Name+".gw";
        string bin = Graph::serializedFilesFolder() + g2Name + (args.bools["-bipartite"] ? "_bipartite" : "_UntypedNodes") 
        + (usingLocks ? "_Locked" : "_Unlocked") + (multi_pairwise ? "_Multi_pairwise" : "_Not_multi_pairwise") + ".bin";
        updateG2 = newerGraphAvailable(gw.c_str(), bin.c_str());
    }
    else
    {
        string bin = Graph::serializedFilesFolder() + g2Name + (args.bools["-bipartite"] ? "_bipartite" : "_UntypedNodes") 
        + (usingLocks ? "_Locked" : "_Unlocked") + (multi_pairwise ? "_Multi_pairwise" : "_Not_multi_pairwise") + ".bin";
        updateG2 = newerGraphAvailable(fg2.c_str(), bin.c_str());
    }
#endif
	
    Timer tLoad;
    tLoad.start();
    thread t1, t2;

#if USE_CACHED_FILES
    if (!updateG1 && !updateG2)
    {
        cout << "Loading graphs using Graph::loadGraphFromBinary()" << endl;
        t1 = thread(Graph::loadGraphFromBinary, ref(G1), g1Name, lockFile, args.bools["-bipartite"], args.bools["-lock-same-names"]);
        t2 = thread(Graph::loadGraphFromBinary, ref(G2), g2Name, lockFile, args.bools["-bipartite"], args.bools["-lock-same-names"]);
    }
    else {
#endif
        if (p1 == 1 && p2 == 1)
        {
            cout << "Loading graphs using Graph::loadGraph()" << endl;
            if(fg1 == "")
            {
                t1 = thread(Graph::loadGraph, g1Name, ref(G1));
            }
            else
            {
                t1 = thread(Graph::loadGraphFromPath, fg1, g1Name, ref(G1), args.doubles["-multipartite"]);
                //Graph::loadFromEdgeListFile(fg1, g1Name, G1, args.bools["-bipartite"]);
            }

            if(fg2 == "")
            {
                t2 = thread(Graph::loadGraph, g2Name, ref(G2));
            }
            else
            {
                t2 = thread(Graph::loadGraphFromPath, fg2, g2Name, ref(G2), args.doubles["-multipartite"]);
                //Graph::loadFromEdgeListFile(fg2, g2Name, G2, args.bools["-bipartite"])
            }
            //G1.maxsize = 4;
            //G2.maxsize = 4;
        }
        else
        {
            cout << "Loading graphs using Graph::multGraph()" << endl;
            t1 = thread(Graph::multGraph, g1Name, p1, ref(G1));
            t2 = thread(Graph::multGraph, g2Name, p2, ref(G2));
        }
#if USE_CACHED_FILES
    }
#endif
    t1.join();
    t2.join();
    cout << "Graph loading completed in " << tLoad.elapsedString() << endl;

    if (G1.getNumNodes() > G2.getNumNodes()) {
        Timer tSwap;
        tSwap.start();
        Graph G3;
        G3 = G1;
        G1 = G2;
        G2 = G3;
        cout << "Switching G1 and G2 because G1 has more nodes than G2. (" << tSwap.elapsedString() << "s)" << endl;
        _graphsSwitched = true;
    }

    Timer T2;
    T2.start();

    double maxGraphletSize = args.doubles["-maxGraphletSize"];
    if (maxGraphletSize){
        G1.setMaxGraphletSize(maxGraphletSize);
        G2.setMaxGraphletSize(maxGraphletSize);
        //std::cout<<"\nSetting max graphlet size to: "<<maxGraphletSize<<endl;
        if (maxGraphletSize == 5 || maxGraphletSize == 4)
            std::cerr<<"Setting maximum graphlet size to "<<maxGraphletSize<<endl;
        else{
            std::cerr<<"ERROR: Invalid graphlet size: "<<maxGraphletSize<<"\nShould be 4 or 5\n";
            exit(0);
        }
    }

    // For "-bipartite"
    if(args.bools["-bipartite"]){
        G1.bipartiteEnabled = true;
        G2.bipartiteEnabled = true;
        cout << "Initializing node types" << endl;

        // Currently we have these constraints
        // if(not (genesG1.size() < genesG2.size())){
        if(not (G1.geneCount <= G2.geneCount)){
            cout << "g1 should have less genes than g2 " << endl;
            cout << "! " <<  G1.geneCount << " < " << G2.geneCount << endl;
            throw runtime_error("g1 should have less genes than g2 \n ");
        }
        // if(not (miRNAsG1.size() < miRNAsG2.size())){
        if(not (G1.miRNACount <= G2.miRNACount)){
            cout << "g1 should have less miRNAs than g2 " << endl;
            cout << "! " <<  G1.miRNACount << " < " << G2.miRNACount << endl;
            throw runtime_error("g1 should have less miRNAs than g2\n ");
        }
    }

    // Getting locks
    if(usingLocks){
        cout << "Initializing locking: " << endl;
        cout << "\t lock file:       " << lockFile << endl;
        cout << "\t lock-same-names: " << args.bools["-lock-same-names"] << endl;

        vector<string> validLocksG1;
        vector<string> validLocksG2;
        unordered_map<string,uint> mapG1 = G1.getNodeNameToIndexMap();
        unordered_map<string,uint> mapG2 = G2.getNodeNameToIndexMap();

        for(uint i = 0; i < column1.size(); i++){
            bool validLock = true;
            string nodeG1 = column1[i];
            string nodeG2 = column2[i];

            if(mapG1.find(nodeG1) == mapG1.end())
                validLock = false;
            if (mapG2.find(nodeG2) == mapG2.end())
                validLock = false;
            if(validLock){
                validLocksG1.push_back(nodeG1);
                validLocksG2.push_back(nodeG2);
            }
        }

        if(args.bools["-lock-same-names"]){
            // iterate each node in G1 and see if same name exists in G2
            int lockedWithSameName = 0;
            for(auto node : mapG1){
                string name = node.first;
                if( mapG2.find(name) != mapG2.end() ){
                    validLocksG1.push_back(name);
                    validLocksG2.push_back(name);
                    lockedWithSameName++;
                }            
            }            
            cout << "Locked " << lockedWithSameName << " nodes since lock-same-nodes is on" << endl; 
        }

        // Setting the locks
        G1.setLockedList(validLocksG1, validLocksG2);
        G2.setLockedList(validLocksG2, validLocksG1);

        if(column1.size() > 0 && column1.size() != validLocksG1.size()){
            cout << "Warning: Out of " << column1.size() << " locks only ";
            cout << validLocksG1.size() << " were valid locks. [Invalid locks are ignored]" << endl;
        }

    }
    cout << "Locking initialization done (" << T2.elapsedString() << ")" << endl;
	
    if (updateG1 || updateG2)
    {
#ifdef REINDEX
        // Method #3 of locking
        Timer tReIndex;
	    cout << "Reindexing graph 1..." << endl;

        tReIndex.start();
        if(args.bools["-bipartite"]){
            G1.reIndexGraph(G1.getBipartiteNodeTypes_ReIndexMap());
        }
        else if(lockFile != ""){
            G1.reIndexGraph(G1.getLocking_ReIndexMap());
        }
        cout << "Done reIndexGraph G1 (" << tReIndex.elapsedString() << ")" << endl;
#endif
        /*double maxSize = args.doubles["-maxGraphletSize"];
        //int maxSize2;
        //stringstream convert(maxS
        if(maxSize){
            std::cout<<"MAXGRAPHLET SIZE IS _____________"<<maxSize<<endl;
            //G1.computeGraphletDegreeVectors(maxSize);

        }*/
#if USE_CACHED_FILES
        cout << "Serializing graphs..." << endl;
        thread t1(Graph::serializeGraph, ref(G1), G1.getName(), G1.bipartiteEnabled, usingLocks);
        thread t2(Graph::serializeGraph, ref(G2), G2.getName(), G2.bipartiteEnabled, usingLocks);
        t1.detach();
        t2.detach();
#endif
    }

    double rewiredFraction1 = args.doubles["-rewire1"];
    if (rewiredFraction1 > 0) {
        if (rewiredFraction1 > 1) {
            throw runtime_error("Cannot rewire more than 100% of G1 edges");
        }
        G1.rewireRandomEdges(rewiredFraction1);
    }

    double rewiredFraction2 = args.doubles["-rewire2"];
    if (rewiredFraction2 > 0) {
        if (rewiredFraction2 > 1) {
            throw runtime_error("Cannot rewire more than 100% of G2 edges");
        }
        G2.rewireRandomEdges(rewiredFraction2);
    }

    if (G1.getNumEdges() == 0 or G2.getNumEdges() == 0) {
        throw runtime_error("One of the networks has 0 edges");
    }
    cout << "Total time for loading graphs (" << T.elapsedString() << ")" << endl;
}
