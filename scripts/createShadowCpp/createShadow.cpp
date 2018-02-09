/*
Sample Run (assuming shadow is the compiled binary)

./shadow -s13276 
    networks/HSapiens/HSapiens.gw networks/SCerevisiae/SCerevisiae.gw networks/RNorvegicus/RNorvegicus.gw 
    HS.oneline.align SC.oneline.align RN.oneline.align > shadow3.gw
*/
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <unordered_map>
#include <stdexcept>
#include "argparse.hpp"

namespace fnv {
    constexpr static unsigned int FNV_PRIME  = 0x01000193;
    constexpr static unsigned int FNV_OFFSET = 0x811c9dc5;
    constexpr unsigned int hash(char const * const str, unsigned int val=FNV_OFFSET) {
        return (str[0] == '\0') ? val : hash(&str[1], (val ^ (unsigned int)str[0]) * FNV_PRIME);
    }
}

enum class GraphType {
        gw,
        el
};

GraphType getGraphType(const char* c_str) {
    std::stringstream err;
    switch (fnv::hash(c_str)) {
        case (fnv::hash(".gw")): return GraphType::gw;
        case (fnv::hash(".el")): return GraphType::el;
        default: { err << "Invalid graphType: " << c_str;
                     std::cerr << err.str().c_str() << std::endl;
                     throw err.str().c_str();
                 }
    }
    return GraphType::gw;
}

namespace filesystem {
    std::string name(const std::string & s) {
        char sep = '/'; // NOT PORTABLE, only on Linux
        std::size_t i = s.rfind(sep, s.length());
        if (i != std::string::npos) {
            return s.substr(i+1);
        }
        return s;
    }
    std::string suffix(const std::string & s) {
        std::size_t dot_pos = s.rfind('.');
        return s.substr(dot_pos);
    }
    std::string stem(const std::string & s) {
        if (s.size() == 0) {
            return std::string("");
        }
        std::size_t dot_pos = s.rfind('.');
        std::size_t slash_pos = s.rfind('/'); // WARNING: NOT PORTABLE
        slash_pos = (slash_pos != std::string::npos) ? slash_pos : 0;
        dot_pos = (dot_pos != std::string::npos) ? dot_pos : std::string::npos;
        return (dot_pos > slash_pos) ? s.substr(slash_pos+1, dot_pos-1-slash_pos) : s.substr(slash_pos+1);
    }
}

namespace shadow_graph {
    class Graph {
        void skipGWheader(std::ifstream & reader) {
            std::string line;
            for (int i = 0; i < 4; i++) {
                std::getline(reader,line);
            }
        }
        void skipGWNodes(std::ifstream & reader, unsigned short numNodes) {
            std::string line;
            for (int i = 0; i < numNodes; i++) {
                std::getline(reader, line);
            }
        }
        void loadGW(std::string filename) {
            std::ifstream reader(filename);
            this->skipGWheader(reader);

            std::string line;
            std::getline(reader,line);
            unsigned short numNodes;
            std::istringstream iss(line);
            iss >> numNodes;
            this->nodes = numNodes;
            this->skipGWNodes(reader, numNodes);

            std::getline(reader,line);
            // Clear iss
            iss.str(line);
            iss.clear();
            int numEdges;
            iss >> numEdges;

            adjList = std::vector<std::vector<Pair> >(numNodes, std::vector<Pair>(0));
            std::cerr << "\tNodes: " << numNodes << ", Edges: " << numEdges << std::endl;
            for (int i = 0; i < numEdges; i++) {
                getline(reader, line);
                iss.str(line);
                iss.clear();
                unsigned short node1;
                unsigned short node2;
                unsigned short weight;
                char dump;
                if (!(iss >> node1 >> node2 >> dump >> dump >> dump)) {
                    std::cerr << "Failed to read edge ( " << i << "th edge) : " << line << std::endl;
                    throw std::runtime_error("EdgeParsingError");
                }
                
                if (!(iss >> weight)) {
                    weight = 1;
                }
                //weight = weight > 0 ? weight : 1;
                node1--;
                node2--;
                this->adjList[node1].push_back(Pair(node2,weight));
                this->adjList[node2].push_back(Pair(node1,weight));
            }
            reader.close();
        }
        void loadEL(std::string filename) {
            std::ifstream reader(filename);
            unsigned short nodes = 0;
            std::string node1;
            std::string node2;
            unsigned short weight;
            std::string line;
            std::istringstream iss;
            while(getline(reader, line)) {
                iss.str(line);
                iss.clear();
                if (!(iss >> node1 >> node2)) {
                    std::cerr << "Failed to read edge: " << line << std::endl;
                    throw std::runtime_error("EdgeParsingError");
                }
                if (!(iss >> weight)) {
                    weight = 1;
                }
                weight = weight > 0 ? weight : 1;
                if (node_map.find(node1) == node_map.end()) {
                    node_map[node1] = nodes++;
                    adjList.push_back(std::vector<Pair>(0));
                }
                if (node_map.find(node2) == node_map.end()) {
                    node_map[node2] = nodes++;
                    adjList.push_back(std::vector<Pair>(0));
                }
                adjList[node_map[node1]].push_back(Pair(node_map[node2],weight));
                adjList[node_map[node2]].push_back(Pair(node_map[node1],weight));
            }
            this->nodes=nodes - 1;
            reader.close();
        }
        public:
            typedef std::pair<unsigned short, unsigned short> Pair;
            std::unordered_map<std::string, unsigned short> node_map;
            std::vector<std::vector<Pair> > adjList;
            unsigned short nodes = 0;
            void clear() {
                node_map.clear();
                for (int i = 0; i < adjList.size(); i++) {
                    adjList[i].clear();
                }
                adjList.clear();
            }
            void load(const std::string filename) {
                this->clear();
                std::string extension = filesystem::suffix(filename);
                GraphType g = getGraphType(extension.c_str());
                if (g == GraphType::gw) {
                    this->loadGW(filename);
                } else {
                    this->loadEL(filename);
                }
            }
    };
    /**
        Expects a one liner format of alignment
        Basically one line text file, and the i-th element index of aligned node of i-th node in shadow network
    */
    void loadAlignment(std::vector<unsigned short> & alignment, std::string & s) {
        std::ifstream reader;
        std::string line;
        reader.open(s);
        std::getline(reader, line);
        std::istringstream iss(line);
        unsigned short nodeNo;
        while (iss >> nodeNo) {
            alignment.push_back(nodeNo);
        }
        reader.close();
    }
}

int main(int argc, const char** argv) {
    static_assert(fnv::hash("FNV Hash Test") == 0xF38B3DB9, "fnv1a_32::hash failure");
    assert(getGraphType(".gw") == GraphType::gw);

    args::ArgumentParser parser("Executable to create the shadow network");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    // args::Flag compact(parser, "compact", "Alignment file format", {'c',"compact"});
    // args::ValueFlag<std::string> format(parser, "format", "Output format", {'f',"format"});
    args::Group required(parser, "", args::Group::Validators::All);
    args::ValueFlag<int> shadowNodeSize(required, "shadowNodeSize", "number of shadowNodes", 
                                            {'s',"shadowNodeSize"});
    args::PositionalList<std::string> networks(required, "networks", "Network filenames");

    try {
        parser.ParseCLI(argc, argv);
    } catch (args::Help) {
        std::cout << parser;
        return 0;
    } catch (args::ParseError e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
    if (args::get(networks).size() % 2 != 0) {
        std::cerr << "Number of alignments must equal number of networks" << std::endl;
        std::cerr << parser;
        return 2;
    }

    // TODO: allow for different alignment file formats
    // bool useCompact = false;
    // if (compact) {
    //     useCompact = true;
    // }
    GraphType g = GraphType::gw;
    // if (format) {
    //     try {
    //         g = getGraphType(args::get(format).c_str());
    //     } catch (char const * e) {
    //         std::cerr << "--format must be el or gw" << std::endl;
    //         return 3;
    //     }
    // }
    std::vector<std::string> pos_args = args::get(networks);
    // Check Existance of files
    for(int i = 0; i < pos_args.size(); i++){
        std::ifstream f(pos_args[i]);
        if(!f.good()){
            std::cerr << "Failed to load file: " << pos_args[i] << std::endl;
            throw std::runtime_error("Network file not found");
        }
        f.close();
    }


    // Separte network and alignment filenames
    std::vector<std::string> network_files(pos_args.size() / 2);
    std::vector<std::string> alignment_files(pos_args.size() / 2);
    int offset = pos_args.size() / 2;
    for (int i = 0; i < (pos_args.size() / 2); i++) {
        network_files[i] = pos_args.at(i);
        alignment_files[i] = pos_args.at(i + offset);
    }
    assert(network_files.size() == alignment_files.size());


    std::vector<std::unordered_map<int, int>> adjList(args::get(shadowNodeSize));
    shadow_graph::Graph tempGraph;
    for (int gi = 0; gi < network_files.size(); gi++) {
        std::cerr << "graph " << gi << ": " << network_files.at(gi) << std::endl;
        //Construct/store graph
        tempGraph.load(network_files.at(gi));
        // Load alignment
        std::vector<unsigned short> tempAlig(0);
        shadow_graph::loadAlignment(tempAlig, alignment_files.at(gi));
        for (int peg = 0; peg < tempAlig.size(); peg++) {
            
            // neighbors of peg
            for (int j = 0; j < tempGraph.adjList[peg].size(); j++) {
                unsigned short end_peg = tempGraph.adjList[peg][j].first;
               
                // only traverse each edge once
                if(peg  == end_peg){
                    std::cerr << "selfloop: " << peg << " " << end_peg << std::endl;
                    throw std::runtime_error("Selfloop");
                }
                if(peg > end_peg) {
                    continue;
                }

                unsigned short hole     = tempAlig.at(peg);
                unsigned short end_hole = tempAlig.at(end_peg);
                
                // we store edge wegiht only one side, from smaller node index to bigger
                if(hole > end_hole)
                    std::swap(hole, end_hole);
                
                adjList[hole][end_hole] += tempGraph.adjList[peg][j].second;

                // debugging
                // if(adjList[hole][end_hole] >= 2){
                    // std::cerr << peg << " " << end_peg << std::endl;
                //     std::cerr << hole << " " << end_hole << " " << adjList[hole][end_hole] << " <- " << tempGraph.adjList[peg][j].second << std::endl;
                //     throw 1;
                // }
            }
        }
    }

    std::cerr << std::endl;
    std::cerr << "outputting shadow graph with nodes = " << adjList.size() << std::endl;

    std::cout << "LEDA.GRAPH" << std::endl;
    std::cout << "string" << std::endl;
    std::cout << "short" << std::endl;
    std::cout << "-2" << std::endl;
    std::cout << adjList.size() << std::endl;
    for (int i = 0; i < adjList.size(); i++) {
        std::cout << "|{shadow" << i << "}|" << std::endl;
    }
    // compute numEdges
    int numEdges = 0;
    for (int i = 0; i < adjList.size(); i++) for(auto it = adjList[i].begin(); it != adjList[i].end(); it++) numEdges++;
    std::cout << numEdges << std::endl;
    for (int i = 0; i < adjList.size(); i++) {
        for (auto it = adjList[i].begin(); it != adjList[i].end(); it++) {
            if (it->first <= i) {
                // this should not happen
                // We only assigned edge value from lower index to higher index nodes
                std::cerr << it->first << " " << i << std::endl;
                throw std::runtime_error("Lost an edge");
                continue;
            }
	    // Stupid LEDA numbers nodes from 1, so +1 to the iterators.
            std::cout << i+1 << ' ' << it->first+1 << " 0 |{" << it->second << "}|" << std::endl;
        }
    }
    return 0;
}
