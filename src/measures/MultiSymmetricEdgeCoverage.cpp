#include "MultiSymmetricEdgeCoverage.hpp"
#include <vector>
#include <iostream>

MultiSymmetricEdgeCoverage::MultiSymmetricEdgeCoverage(vector<Graph>* GV, Graph* SN,string measureName):
    Measure(measureName), graphs(GV), shadowNetwork(SN)  {

	shadowNetwork->getAdjMatrix(SNAdjMatrix);
	AdjMatrices = vector<vector<vector<bool> >>(0);

	vector<vector<bool> > tempAdjMatrix;
    for (uint i = 0; i < (*GV).size(); i++) {
        (*GV)[i].getAdjMatrix(tempAdjMatrix);
        AdjMatrices.push_back(tempAdjMatrix);
	}
}

MultiSymmetricEdgeCoverage::MultiSymmetricEdgeCoverage():Measure("msec"){};//dummy declare

MultiSymmetricEdgeCoverage::~MultiSymmetricEdgeCoverage() {
}

ushort MultiSymmetricEdgeCoverage::weight(vector<vector<ushort> >& A,ushort n1, ushort n2) {
    ushort empty = 0-1;
    uint count = 0;
    //cerr<<"check at the begining of MultiSymmetricEdgeCoverage::weight @MultiSymmetricEdgeCoverage.cpp"<<endl;

    count+=SNAdjMatrix[n1][n2];
    for (uint i = 0; i < A.size(); i++) {
		if((A[i][n1]!=empty)&&(A[i][n2]!=empty)){
		    count+=AdjMatrices[i][A[i][n1]][A[i][n2]];
		}
	}
    //cerr<<"check at the end of MultiSymmetricEdgeCoverage::weight @MultiSymmetricEdgeCoverage.cpp"<<endl;

	return count;

}
double MultiSymmetricEdgeCoverage::eval(const MultiAlignment& A) {

    int numerator = 0;
    uint temp;
    vector<vector<ushort> > mapping = A.getUpAlign();
    //cerr<<"SNAdjMatrix.size() = "<<SNAdjMatrix.size()<<endl;
    //cerr<<"SNAdjMatrix[0].size() = "<<SNAdjMatrix[0].size()<<endl;
    //cerr<<"check at the begining of MultiSymmetricEdgeCoverage::eval @MultiSymmetricEdgeCoverage.cpp"<<endl;
    for(ushort n1 = 0;n1<shadowNetwork->getNumNodes()-1;n1++){
	    for(ushort n2 = n1+1;n2< shadowNetwork->getNumNodes();n2++){
			//cerr<<"check before A.weight with n1="<<n1<<" n2="<<n2<<endl;
			temp = weight(mapping,n1, n2);
			if(temp > 1) numerator+=temp;
		}
	}
    //cerr<<"check after counting all the edges @MultiSymmetricEdgeCoverage.cpp"<<endl;

	int denominator = 0;
    for(uint i = 0;i<(*graphs).size();i++ ){
		denominator += (*graphs)[i].getNumEdges();
	}
	//cerr<<"check after getting the denominator @MultiSymmetricEdgeCoverage.cpp"<<endl;
	denominator += shadowNetwork->getNumEdges();
    /*std::cout<<"\n\n-sec print start";
    std::cout<<"\nA.numAlignedEdges(*G1, *G2) = "<<A.numAlignedEdges(*G1, *G2)<<"  G1->getNumEdges() = "<<G1->getNumEdges();
    std::cout<<"\nA.numAlignedEdges(*G2, *G1) = "<<A.numAlignedEdges(*G2, *G1);
    std::cout<<"  G2->getNumEdges() = "<<G2->getNumEdges();
    std::cout<<"\nresult = "<<result<<"\n\n";*/

    return (double) numerator/denominator;
}

ushort MultiSymmetricEdgeCoverage::getNumerator(const MultiAlignment& A) {

    uint numerator = 0;
    uint temp;
    vector<vector<ushort> > mapping = A.getUpAlign();
    //cerr<<"SNAdjMatrix.size() = "<<SNAdjMatrix.size()<<endl;
    //cerr<<"SNAdjMatrix[0].size() = "<<SNAdjMatrix[0].size()<<endl;
    //cerr<<"check at the begining of MultiSymmetricEdgeCoverage::eval @MultiSymmetricEdgeCoverage.cpp"<<endl;
    for(ushort n1 = 0;n1<shadowNetwork->getNumNodes()-1;n1++){
	    for(ushort n2 = n1+1;n2< shadowNetwork->getNumNodes();n2++){
			//cerr<<"check before A.weight with n1="<<n1<<" n2="<<n2<<endl;
			temp = weight(mapping,n1, n2);
			if(temp > 1) numerator+=temp;
		}
	}
    //cerr<<"check after counting all the edges @MultiSymmetricEdgeCoverage.cpp"<<endl;
    return numerator;
}

ushort MultiSymmetricEdgeCoverage::getDenominator(const MultiAlignment& A) {

	uint denominator = 0;
    for(uint i = 0;i<(*graphs).size();i++ ){
		denominator += (*graphs)[i].getNumEdges();
	}
	//cerr<<"check after getting the denominator @MultiSymmetricEdgeCoverage.cpp"<<endl;
	denominator += shadowNetwork->getNumEdges();
    /*std::cout<<"\n\n-sec print start";
    std::cout<<"\nA.numAlignedEdges(*G1, *G2) = "<<A.numAlignedEdges(*G1, *G2)<<"  G1->getNumEdges() = "<<G1->getNumEdges();
    std::cout<<"\nA.numAlignedEdges(*G2, *G1) = "<<A.numAlignedEdges(*G2, *G1);
    std::cout<<"  G2->getNumEdges() = "<<G2->getNumEdges();
    std::cout<<"\nresult = "<<result<<"\n\n";*/

    return denominator;
}

double MultiSymmetricEdgeCoverage::eval(const Alignment& A) {return 0;};//dummy declare