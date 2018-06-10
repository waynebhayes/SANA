//
// Created by taylor on 3/27/18.
//

#include <fstream>
#include <sstream>
#include <set>
#include <Timer.hpp>
#include "Utils.hpp"
#include "GoAverage.hpp"
#include "GoSimilarity.hpp"

GoAverage::GoAverage(Graph *G1, Graph *G2) : Measure(G1,G2,"goavg") {}

GoAverage::~GoAverage() {}

double GoAverage::eval(const Alignment &A) {
    string G1Name = G1->GetName();
    string G2Name = G2->GetName();

    string G1GoFile = "../../../networks/"+G1Name+"/go/"+G2Name+"_gene_association.txt";
    string G2GoFile = "../../../networks/"+G2Name+"/go/"+G1Name+"_gene_association.txt";

    string G1GoSimpleFile = GoSimilarity::getGoSimpleFileName(*G1);
    string G2GoSimpleFile = GoSimilarity::getGoSimpleFileName(*G2);

    GoSimilarity::ensureGoFileSimpleFormatExists(*G1);
    GoSimilarity::ensureGoFileSimpleFormatExists(*G2);

    string alignmentFile = G1Name+"_"+G2Name+"_"+"GoAverageAlignment.aln";
    ofstream outfile(alignmentFile);
    A.dumpEdgeList(*G1,*G2,outfile);
    outfile.close();

    string pairwiseOutputFile = G1Name+"_"+G2Name+"_GoAverageTermPairs.trm";

    /* PAIRWISE BEGIN */
    /* Script Params = alignFile pairwiseOutputFile */

    ifstream goterms1(G1GoSimpleFile);
    ifstream goterms2(G2GoSimpleFile);
    ifstream aln(alignmentFile);
    ofstream out(pairwiseOutputFile);

    unordered_map < string,vector<string> > genegoterms1; //Our container for go stuff
    string token_str; //Our string to parse stuff
    while(getline(goterms1,token_str)) {
        stringstream ss(token_str);
        string gene, term;
        ss >> gene;
        ss >> term;
        auto needle = genegoterms1.find(gene);
        if(needle != genegoterms1.end()) {
            genegoterms1[gene].push_back(term);
        } else {
            genegoterms1.emplace(gene,vector<string>());
        }
    }

    unordered_map < string,vector<string> > genegoterms2; //Our container for go stuff 2
    while(getline(goterms2,token_str)) {
        stringstream ss(token_str);
        string gene, term;
        ss >> gene;
        ss >> term;
        auto needle = genegoterms2.find(gene);
        if(needle != genegoterms2.end()) {
            genegoterms1[gene].push_back(term);
        } else {
            genegoterms1.emplace(gene,vector<string>());
        }
    }

    set < pair<string,string> > termpairs;
    while(getline(aln,token_str)) {
        stringstream ss(token_str);
        string pair_0, pair_1;

        ss >> pair_0;
        ss >> pair_1;

        auto needle_1 = genegoterms1.find(pair_0);
        auto needle_2 = genegoterms2.find(pair_1);

        vector <string> terms1;
        vector <string> terms2;

        if(needle_1 != genegoterms1.end()) {
            terms1 = genegoterms1[pair_0];
        } else { terms1 = {}; }

        if(needle_2 != genegoterms2.end()) {
            terms2 = genegoterms2[pair_1];
        } else { terms2 = {}; }

        for(auto i : terms1) {
            for(auto j : terms2) {
                termpairs.insert(make_pair(i,j));
            }
        }
    }

    for(auto i : termpairs) {
        out << get<0>(i) << "\t" << get<1>(i) << "\n";
    }
    out.close();
    /* PAIRWISE END */

    /* ITGOM BEGIN */
    Timer t;
    string itgomScript = "go/itgom.py";

    t.start();
    cout << "Executing " << itgomScript << endl;
    //python itgom.py term_pairs.trm
    Utils::execPrintOutput("python "+itgomScript+" "+pairwiseOutputFile);
    cout << "Done (" << t.elapsedString() << ")" << endl;

    string simScript = "go/protein_pair_sim.py";

    string itgomScriptOutputFile = G1Name+"_"+G2Name+"_GoAverageTermPairs.sim";
    string simScriptOutputFile = G1Name+"_"+G2Name+"_GoAverageSim.txt";

    string simScriptParameters = G1GoSimpleFile+" "+G2GoSimpleFile;
    simScriptParameters+=" "+alignmentFile+" "+itgomScriptOutputFile+" "+simScriptOutputFile;

    t.start();
    cout << "Executing " << simScript << endl;
    //python protein_pair_sim.py terms1.txt terms2.txt alignment.aln file.sim output.txt
    string result = Utils::exec("python "+simScript+" "+simScriptParameters);
    cout << "Done (" << t.elapsedString() << ")" << endl;

    double score = stod(result);
    /* ITGOM END */

    return 0;
}
