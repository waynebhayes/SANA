#include "ConservedInteractionQuality.hpp"
#include <vector>
#include <iostream>
#include <cassert>

ConservedInteractionQuality::ConservedInteractionQuality(vector<Graph>* GV, Graph* SN,string measureName):
    Measure(measureName), graphs(GV), shadowNetwork(SN)  {

	shadowNetwork->getAdjMatrix(SNAdjMatrix);
	AdjMatrices = vector<vector<vector<bool> >>(0);

	vector<vector<bool> > tempAdjMatrix;
    for (uint i = 0; i < (*GV).size(); i++) {
        (*GV)[i].getAdjMatrix(tempAdjMatrix);
        AdjMatrices.push_back(tempAdjMatrix);
	}
}

ConservedInteractionQuality::ConservedInteractionQuality():Measure("ciq"){};//dummy declare

ConservedInteractionQuality::~ConservedInteractionQuality() {
}

double ConservedInteractionQuality::weight(vector<vector<ushort> >& A,ushort n1, ushort n2) {
    ushort empty = 0-1;
    uint denominator = 0;
    uint count = 0;
    //cerr<<"check at the begining of ConservedInteractionQuality::weight @ConservedInteractionQuality.cpp"<<endl;

    count += SNAdjMatrix[n1][n2];
    denominator += 1;
    for (uint i = 0; i < A.size(); i++) {
		if((A[i][n1]!=empty)&&(A[i][n2]!=empty)){
			denominator += 1;
		    count+=AdjMatrices[i][A[i][n1]][A[i][n2]];
		}
	}
    //cerr<<"check at the end of ConservedInteractionQuality::weight @ConservedInteractionQuality.cpp"<<endl;
    double result = (count > 1) ? count*count/denominator : 0 ;
	return result;

}

double ConservedInteractionQuality::eval(const MultiAlignment& A) {

    double numerator = 0;
    vector<vector<ushort> > mapping = A.getUpAlign();
    //cerr<<"SNAdjMatrix.size() = "<<SNAdjMatrix.size()<<endl;
    //cerr<<"SNAdjMatrix[0].size() = "<<SNAdjMatrix[0].size()<<endl;
    //cerr<<"check at the begining of ConservedInteractionQuality::eval @ConservedInteractionQuality.cpp"<<endl;
    for(ushort n1 = 0;n1<shadowNetwork->getNumNodes()-1;n1++){
	    for(ushort n2 = n1+1;n2< shadowNetwork->getNumNodes();n2++){
			//cerr<<"check before A.weight with n1="<<n1<<" n2="<<n2<<endl;
			numerator+=weight(mapping,n1, n2);
		}
	}

	//cerr<<"check after getting the denominator @ConservedInteractionQuality.cpp"<<endl;
	int denominator = 0;
    for(uint i = 0;i<(*graphs).size();i++ ){
		denominator += (*graphs)[i].getNumEdges();
	}
    //cerr<<"check after counting all the edges @ConservedInteractionQuality.cpp"<<endl;
	denominator += shadowNetwork->getNumEdges();

    /*std::cout<<"\n\n-sec print start";
    std::cout<<"\nA.numAlignedEdges(*G1, *G2) = "<<A.numAlignedEdges(*G1, *G2)<<"  G1->getNumEdges() = "<<G1->getNumEdges();
    std::cout<<"\nA.numAlignedEdges(*G2, *G1) = "<<A.numAlignedEdges(*G2, *G1);
    std::cout<<"  G2->getNumEdges() = "<<G2->getNumEdges();
    std::cout<<"\nresult = "<<result<<"\n\n";*/

    return (double) numerator/denominator;
}

double ConservedInteractionQuality::getNumerator(const MultiAlignment& A) {

    double numerator = 0;
    vector<vector<ushort> > mapping = A.getUpAlign();
    //cerr<<"SNAdjMatrix.size() = "<<SNAdjMatrix.size()<<endl;
    //cerr<<"SNAdjMatrix[0].size() = "<<SNAdjMatrix[0].size()<<endl;
    //cerr<<"check at the begining of ConservedInteractionQuality::eval @ConservedInteractionQuality.cpp"<<endl;
    for(ushort n1 = 0;n1<shadowNetwork->getNumNodes()-1;n1++){
	    for(ushort n2 = n1+1;n2< shadowNetwork->getNumNodes();n2++){
			//cerr<<"check before A.weight with n1="<<n1<<" n2="<<n2<<endl;
			numerator+=weight(mapping,n1, n2);
		}
	}
    //cerr<<"check after counting all the edges @ConservedInteractionQuality.cpp"<<endl;
    return numerator;
}

ushort ConservedInteractionQuality::getDenominator(const MultiAlignment& A) {

	//cerr<<"check after getting the denominator @ConservedInteractionQuality.cpp"<<endl;
	int denominator = 0;
    for(uint i = 0;i<(*graphs).size();i++ ){
		denominator += (*graphs)[i].getNumEdges();
	}
    //cerr<<"check after counting all the edges @ConservedInteractionQuality.cpp"<<endl;
	denominator += shadowNetwork->getNumEdges();

    return denominator;
}

ushort ConservedInteractionQuality::getWeightDenominator(const vector<vector<ushort> >& A,ushort n1,ushort n2) {
    ushort empty = 0-1;
    ushort denominator = 1;
    //cerr<<"check at the begining of ConservedInteractionQuality::getWeightDenominator @ConservedInteractionQuality.cpp"<<endl;

    for (uint i = 0; i < A.size(); i++) {
		if((A[i][n1]!=empty)&&(A[i][n2]!=empty)){
			denominator += 1;
		}
	}
    //cerr<<"check at the end of ConservedInteractionQuality::getWeightDenominator @ConservedInteractionQuality.cpp"<<endl;
	return denominator;

}

ushort ConservedInteractionQuality::getWeightNumerator(const vector<vector<ushort> >& A,ushort n1,ushort n2) {
    ushort empty = 0-1;
    ushort numerator = 0;
    //cerr<<"check at the begining of ConservedInteractionQuality::getWeightNumerator @ConservedInteractionQuality.cpp"<<endl;
    assert(n1 != n2);
    numerator += SNAdjMatrix[n1][n2];
    for (uint i = 0; i < A.size(); i++) {
		if((A[i][n1]!=empty)&&(A[i][n2]!=empty)){
		    numerator += AdjMatrices[i][A[i][n1]][A[i][n2]];
		}
	}
    //cerr<<"check at the end of ConservedInteractionQuality::getWeightNumerator @ConservedInteractionQuality.cpp"<<endl;
	//return (numerator > 1) ? numerator : 0 ;
	return numerator;
}


double ConservedInteractionQuality::eval(const Alignment& A) {return 0;};//dummy declare