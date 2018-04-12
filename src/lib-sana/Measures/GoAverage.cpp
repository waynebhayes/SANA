//
// Created by taylor on 3/27/18.
//

#include <fstream>
#include "GoAverage.hpp"
#include "GoSimilarity.hpp"

GoAverage::GoAverage(Graph *G1, Graph *G2) : Measure(G1,G2,"goavg") {}

GoAverage::~GoAverage() {}

double GoAverage::eval(const Alignment &A) {
    string G1Name = G1->GetName();
    string G2Name = G2->GetName();

    string G1GoFile = "../../../networks/"+G1Name+"/go/"+G2Name+"_gene_association.txt";
    string G2GoFile = "../../../networks/"+G2Name+"/go/"+G1Name+"_gene_association.txt";

    //TODO: Implement helper function to retrieve "Simple Go File Name"
    string G1GoSimpleFile = GoSimilarity::getGoSimpleFileName(*G1);
    string G2GoSimpleFile = GoSimilarity::getGoSimpleFileName(*G2);

    //TODO: Assert Simple File Exists and Format is Correct
    //Assert(G1GoSimpleFile Exists)

    string alignmentFile = G1Name+"_"+G2Name+"_"+"GoAverageAlignment.aln";
    ofstream outfile(alignmentFile);
    A.dumpEdgeList(*G1,*G2,outfile);
    outfile.close();




    return 0;
}
