/* recordstore.h
Author: Jialu Hu
Date: 28.06.2012*/

#ifndef RECORDSTORE_H_
#define RECORDSTORE_H_

#include <lemon/core.h>
#include <lemon/bits/graph_extender.h>
#include <string>
#include <array>
#include <set>
#include <unordered_map>
#include <random>
#include <chrono>
#include "macro.h"
#include "verbose.h"
#include <cmath>
#include "function.h"

template<typename KpGraph,typename Option>
class RecordStore
{
private:
  typedef std::array<float, MAXIMUM_RECORDS> NodeScores;
  typedef std::array<unsigned, MAXIMUM_RECORDS> StrScores;// Structural score for each record.
  typedef std::array<float, NUM_PRO_BARS> Distribution;
  typedef std::array<double, NUM_PRO_BARS> DistriBar;
  typedef typename KpGraph::NetwookPool_Type NetworkPool;
  typedef typename NetworkPool::Graph Graph;
  typedef typename NetworkPool::OrigLabelNodeMap OrigLabelNodeMap;
  typedef typename NetworkPool::InvOrigLabelNodeMap InvOrigLabelNodeMap;
  typedef typename KpGraph::BpGraph BpGraph;
  
  typedef struct _ValueType
  {
    std::string protein;
    double evalue;
    _ValueType(std::string pro, double e)
    {
      protein=pro;
      evalue=e;
    }
  }ValueType;
  TEMPLATE_GRAPH_TYPEDEFS(Graph);
public:
  typedef KpGraph KpGraphType;
  typedef typename KpGraph::EdgeMap EdgeMap;
  typedef typename Graph::template EdgeMap<float> EdgeWeight;
  typedef std::array<std::string, MAXIMUM_RECORDS> Proteins;
  typedef Proteins* RecordArray;
  typedef std::unordered_map<std::string,short> ArraySpeciesMap;
  typedef std::unordered_map<std::string,float> ArrayScoreMap;
  //typedef std::unordered_map<std::string,float> ArrayNodeScoreMap;
  //typedef std::unordered_map<std::string,unsigned> ArrayStrScoreMap;
  typedef struct _MatchingEdge
  {
    Proteins* recordArray;
    ArraySpeciesMap speciesMap;
    ArrayScoreMap scoreMap;
    //ArrayStrScoreMap  strScoreMap;
    unsigned length;
    _MatchingEdge():length(0)
    {
      recordArray = new Proteins[2];
    }
    ~_MatchingEdge()
    {
      delete [] recordArray;
    }
    std::string getValue(unsigned i,unsigned select)
    {
      return recordArray[i][select];
    }
    short getSpecies(std::string protein)
    {
      return speciesMap[protein];
    }
    float getMatchingScore(std::string protein)
    {
      return scoreMap[protein];
    }
    bool isMatchingEdge(std::string& keystring)
    {
      return ( scoreMap.find(keystring) != scoreMap.end() );
    }
    bool addValue(std::string& protein1,
                  std::string& protein2,
                  short num1,
                  short num2,
                  float score)
    {
      if(length==MAXIMUM_RECORDS)
         return false;
      if( num1==num2 && protein1.compare(protein2)>0 )
      {
		  std::string temp;
		  temp = protein1;
		  protein1=protein2;
		  protein2=temp;
	  }
      std::string keystring(protein1);
      keystring.append(protein2);
      recordArray[0][length] = protein1;
      recordArray[1][length] = protein2;
      speciesMap[protein1] = num1;
      speciesMap[protein2] = num2;
      scoreMap[keystring]   = score;
      length++;
      return true;
    }
  }MatchingEdge; 
  typedef struct _TripletArray
  {
    Proteins* recordArray;
    ArraySpeciesMap speciesMap;
    ArrayScoreMap scoreMap;
    //ArrayStrScoreMap  strScoreMap;
    unsigned length;
    _TripletArray():length(0)
    {
      recordArray = new Proteins[3];
    }
    ~_TripletArray()
    {
      delete [] recordArray;
    }
    std::string getValue(unsigned i,unsigned select)
    {
      return recordArray[i][select];
    }
    short getSpecies(std::string protein)
    {
      return speciesMap[protein];
    }
    float getTripletScore(std::string protein)
    {
      return scoreMap[protein];
    }
    bool addValue(std::string protein1,
                  std::string protein2,
                  std::string protein3,
                  short num,
                  float score)
    {
      if(length==MAXIMUM_RECORDS)
         return false;
      recordArray[0][length] = protein1;
      recordArray[1][length] = protein2;
      recordArray[2][length] = protein3;
      speciesMap[protein3] = num;
      protein1.append(protein3);//Protein1Protein3 is unique in the record dataset.
      scoreMap[protein1]   = score;
      length++;
      return true;
    }
  }TripletArray; 
  typedef struct _RecordArrayValue
  {
    Proteins* recordArray;
    NodeScores* nodeScores;
    StrScores* strScores;
    unsigned length;
    _RecordArrayValue()
    {
      recordArray=new Proteins[3];//triple nodes
      nodeScores=new NodeScores();
      strScores=new StrScores();
      length=0;
    }
    ~_RecordArrayValue()
    {
      delete [] recordArray;
      delete nodeScores;
      delete strScores;
    }
  }RecordArrayValue;
  typedef std::array<RecordArrayValue*,NUM_SPECIES-2> RecordArrayArray;
  typedef std::unordered_multimap<std::string,ValueType> MapType;
  typedef std::unordered_map<std::string,double> RecordHashMap;
  //RecordArray recordarray;
  //RecordArrayArray recordarrayarray;
  //TripletArray* tripletRecord;
  MatchingEdge* matchingEdges;
  Proteins* proteins;
  Proteins* proteins1;
  Proteins* proteins2;
  Proteins* proteins3;
  Distribution* nullDistr;
  Distribution* homoDistr;
  Distribution* resultDistr;
  DistriBar* distriBar;
  //NodeScores* nodeScores;
  //StrScores* strScores;
  std::string hfile; /// filename of homology model
  std::string nfile; /// filename of null model
  std::string rfile; ///filename of nodescore distribution with respect of evalue
  std::string recordfile; /// filename of alignment records
  std::string resultfolder;
  unsigned length;
  unsigned numSpecies;
	int numThreads;
  float maxNodeScore;
  float minNodeScore;
  unsigned maxStrScore;
  unsigned minStrScore;
  double beta;
  unsigned seed;
  float alpha;
  bool bscore;
  std::default_random_engine generator;
  std::uniform_real_distribution<float> distribution;
  std::vector<Graph*> bpgraphs;
  std::vector<OrigLabelNodeMap*> node2labelVector;
  std::vector<InvOrigLabelNodeMap*> label2nodesVector;
  std::vector<EdgeWeight*> edgemapVector;
  
  RecordStore(std::string&,std::string&,Option&);
  ~RecordStore();
  bool initialBpGraphs();
  bool deleteBpGraphs();
  bool readRecords(const char*);
  bool _readRecords(const char*);
  //bool readRecords2(const char*);
  std::string getRecords(unsigned, unsigned);
  //std::string _getRecords(unsigned, unsigned);
  float getRecordScore(unsigned);
  float _getRecordSeScore(unsigned);
  float _getRecordStScore(unsigned);
  bool createRecords(const char*,const char*,const char*);
  bool readHomoList(MapType&, const char*,std::unordered_map<std::string,double>&);
  bool readHomoList2(MapType&, const char*,std::unordered_map<std::string,double>&,
  InvOrigLabelNodeMap*,InvOrigLabelNodeMap*);
  bool readDistribution(const char*,const char*);
  float scoreCalculate(double,double,double);
  float getScore(double);
  float getBitScore(float);
  bool readScore(const char*);
  bool createBpGraph(KpGraph&,NetworkPool&,Graph*,OrigLabelNodeMap*,                                InvOrigLabelNodeMap*, EdgeWeight*,int,int,int);
	bool assignEdgeScore(Graph*,OrigLabelNodeMap*,EdgeWeight*,BpGraph*,bool,std::ofstream&);
  bool createBpGraphAll(KpGraph&,NetworkPool&);
  bool _createRecords(KpGraph&, NetworkPool&);
  /// bool createRecords_t(KpGraph&, NetworkPool&);
  bool createRecords_MNetAligner(std::vector<std::string>&,NetworkPool&);
  bool getInvolvedRecord(std::string&,
                         std::string*,
                         unsigned,MapType*,
                         std::ofstream&,
                         RecordHashMap*);
  bool printRecord(std::ofstream&,std::string*,float);
  bool _scoreCalculate(std::string*,RecordHashMap*,float&);
  float combineScore(float, unsigned);
  void top10000(std::string&);
};

template<typename KpGraph,typename Option>
RecordStore<KpGraph,Option>::RecordStore(std::string& filename1,
                              std::string& filename2,
                              Option& myoption)
:
//recordarrayarray(),
//tripletRecord(NULL),
matchingEdges(NULL)
,length(0)
,numSpecies(myoption.numspecies)
,numThreads(myoption.numthreads)
,maxNodeScore(0.0)
,minNodeScore(0.0)
,maxStrScore(0)
,minStrScore(0)
,beta(myoption.beta)
,seed(std::chrono::system_clock::now().time_since_epoch().count())
,alpha(myoption.alpha)
,bscore(myoption.bscore)
,generator(seed)
,distribution(0.0,1.0)
//,graphs()
//,node2label(gr)
//,label2node()
//,edgemap(gr)
{
  hfile = filename1;
  nfile = filename2;
  rfile = myoption.scorefile;
  recordfile = myoption.recordsfile;
  resultfolder = myoption.resultfolder;
  nullDistr = new Distribution();
  homoDistr = new Distribution();
  resultDistr = new Distribution();
  distriBar = new DistriBar();
  //strScores = new StrScores();
  //nodeScores = new NodeScores();
  //recordarray = new Proteins[numSpecies];
  initialBpGraphs();
}

template<typename KpGraph,typename Option>
RecordStore<KpGraph,Option>::~RecordStore()
{
  //delete nodeScores;
  //delete strScores;
  delete nullDistr;
  delete homoDistr;
  delete distriBar;
  delete resultDistr;
  //delete [] recordarray;
  deleteBpGraphs();
}

template<typename KpGraph,typename Option>
bool
RecordStore<KpGraph,Option>::initialBpGraphs()
{
  for(unsigned i=0;i<numSpecies;i++)
	for(unsigned j=i;j<numSpecies;j++)
	  {
	    Graph* mygraph = new Graph();
	    OrigLabelNodeMap* mynode2lable = new OrigLabelNodeMap(*mygraph);
	    InvOrigLabelNodeMap* mylabel2node = new InvOrigLabelNodeMap();
	    EdgeWeight* myedgemap = new EdgeWeight(*mygraph);
	    bpgraphs.push_back(mygraph);
	    node2labelVector.push_back(mynode2lable);
	    label2nodesVector.push_back(mylabel2node);
	    edgemapVector.push_back(myedgemap);
	  }
  return true;
}

template<typename KpGraph,typename Option>
bool
RecordStore<KpGraph,Option>::deleteBpGraphs()
{
  int numBpGraphs = numSpecies*(numSpecies-1)/2;
  for(int i=0;i<numBpGraphs;i++)
  {
    delete bpgraphs[i];
    delete node2labelVector[i];
    delete label2nodesVector[i];
    delete edgemapVector[i];
  }
  return true;
}

//template<typename KpGraph,typename Option>
//bool
//RecordStore<KpGraph,Option>::readRecords(const char* filename)
//{
//  std::ifstream input(filename);
//  std::string line;
//  if(!input.good())
//  {
//    std::cerr << filename <<"cannot be opened!"<<std::endl;
//    return 1;
//  }
//
//  nodeScores = new NodeScores();
//  strScores = new StrScores();
//  unsigned int i=0;
//  while(std::getline(input,line))
//  {
//    std::stringstream lineStream(line);
//    lineStream >> (*proteins1)[i] >> (*proteins2)[i] >> (*proteins3)[i] >> (*nodeScores)[i];
//    i++;
//  }
//  return 0;
//}

//template<typename KpGraph,typename Option>
//bool
//RecordStore<KpGraph,Option>::readRecords2(const char* filename)
///// read records for M-NetAligner on multiple networks
//{
//  std::ifstream input(filename);
//  std::string line;
//  if(!input.good())
//  {
//    std::cerr << filename <<" cannot be opened!"<<std::endl;
//    return 1;
//  }
//  unsigned int i=0;
//  bool isFirstLine=true;
//  while(std::getline(input,line))
//  {
//    std::stringstream lineStream(line);
//    for(unsigned k=0;k<numSpecies;k++)
//    {
//      lineStream >> recordarray[k][i];
//    }
//    lineStream >> (*nodeScores)[i];/// no score for structural similarity.
//    if(isFirstLine)
//    {
//      maxNodeScore=(*nodeScores)[i];
//      minNodeScore=(*nodeScores)[i];
//      isFirstLine=false;
//    }
//    if((*nodeScores)[i]>maxNodeScore)
//      maxNodeScore=(*nodeScores)[i];
//    if((*nodeScores)[i]<minNodeScore)
//      minNodeScore=(*nodeScores)[i];
//    i++;
//    if(i >= MAXIMUM_RECORDS)
//    {
//      if(g_verbosity>=VERBOSE_DEBUG)
//        std::cerr<<"The alingment records exceeds the maximal limit!"<<std::endl;
//      break;
//    }
//  }
//  length=i;
//  std::cout <<length<<std::endl;
//  return true;
//}

//template<typename KpGraph,typename Option>
//bool
//RecordStore<KpGraph,Option>::_readRecords(const char* filename)
//{
//  std::ifstream input(filename);
//  std::string line;
//  if(!input.good())
//  {
//    std::cerr << filename <<" cannot be opened!"<<std::endl;
//    return 1;
//  }
//  unsigned int i=0;
//  bool isFirstLine=true;
//  while(std::getline(input,line))
//  {
//    std::stringstream lineStream(line);
//    for(unsigned k=0;k<numSpecies;k++)
//    {
//      lineStream >> recordarray[k][i];
//    }
//    lineStream >> (*nodeScores)[i] >> (*strScores)[i];
//    if(isFirstLine)
//    {
//      maxNodeScore=(*nodeScores)[i];
//      minNodeScore=(*nodeScores)[i];
//      maxStrScore=(*strScores)[i];
//      minStrScore=(*strScores)[i];
//      isFirstLine=false;
//    }
//    if((*nodeScores)[i]>maxNodeScore)
//      maxNodeScore=(*nodeScores)[i];
//    if((*nodeScores)[i]<minNodeScore)
//      minNodeScore=(*nodeScores)[i];
//    if((*strScores)[i]>maxStrScore)
//      maxStrScore=(*strScores)[i];
//    if((*strScores)[i]<minStrScore)
//      minStrScore=(*strScores)[i];
//    i++;
//  }
//  length=i;
//  return true;
//}

//template<typename KpGraph,typename Option>
//std::string
//RecordStore<KpGraph,Option>::_getRecords(unsigned select, unsigned species)
//{
//  return recordarray[species][select];
//}

/*template<typename KpGraph,typename Option>
std::string
RecordStore<KpGraph,Option>::getRecords(unsigned select, unsigned species)
{
  std::string protein;
  switch(species)
  {
    case 0:protein=proteins1->at(select);break;
    case 1:protein=proteins2->at(select);break;
    case 2:protein=proteins3->at(select);break;
    default: ;
  }
  return protein;
}*/

//template<typename KpGraph,typename Option>
//float
//RecordStore<KpGraph,Option>::getRecordScore(unsigned select)
//{
//  return nodeScores->at(select);
//}
//template<typename KpGraph,typename Option>
//float
//RecordStore<KpGraph,Option>::_getRecordSeScore(unsigned select)
//{
//  return (nodeScores->at(select)-minNodeScore)/(maxNodeScore-minNodeScore);
//}

/*
template<typename KpGraph,typename Option>
float
RecordStore<KpGraph,Option>::_getRecordStScore(unsigned select)
{
  float temp = pow(static_cast<double>(strScores->at(select)-minStrScore)/(maxStrScore-minStrScore),FACTOR_EDGE);
  return temp;
}*/

template<typename KpGraph,typename Option>
bool
RecordStore<KpGraph,Option>::createBpGraphAll(KpGraph& kpgraph,NetworkPool& networkpool)
{
  std::string filename(resultfolder);
  filename.append("scoreRecords.txt");
  std::ofstream output(filename.c_str());
	std::cout << "Reweighting started!" << std::endl;
  kpgraph.reweightingAll(networkpool,numThreads);
	std::cout << "Reweighting finished!" << std::endl;
  readScore(rfile.c_str());
	maxStrScore = kpgraph.maxStrWeight;/// 6551
	int ni,nj,numBp;
	ni=0;nj=-1;
	numBp=numSpecies*(numSpecies+1)/2;

	std::cout << "CreateBpGraph started!" << std::endl;
//#pragma omp parallel for num_threads(numThreads) shared(ni,nj) schedule(dynamic,1)// have serious problem
	for(int i=0;i<numBp;i++)
	{
		int lni,lnj;
		#pragma omp critical
		{
			if(nj<static_cast<int>(numSpecies)-1)nj++;
			else{ni++;nj=ni;}
			lni=ni;
			lnj=nj;
		}
		createBpGraph(kpgraph,networkpool,
                    bpgraphs[i],
                    node2labelVector[i],
                    label2nodesVector[i],
                    edgemapVector[i],lni,lnj,i);
        BpGraph* bp12=kpgraph.graphs[i];
				bool isHomoNet(false);
				if(lni==lnj)isHomoNet=true;
				assignEdgeScore(bpgraphs[i],node2labelVector[i],edgemapVector[i],bp12,isHomoNet,output);
	}      
	std::cout << "CreateBpGraph finished!" << std::endl;
	output.close();
  return true;
}

template<typename KpGraph,typename Option>
bool
RecordStore<KpGraph,Option>::assignEdgeScore(Graph* gr,
											OrigLabelNodeMap* node2label,
											EdgeWeight* edgemap,
											BpGraph* bp12,
											bool isHomoNet,
											std::ofstream& output)
{
	Node node1,node2;
	std::string protein1,protein2,kst;
	float sescore,weight,serange,sePart,stPart;
	unsigned stWeight;
	//int ind;
	serange=maxNodeScore-minNodeScore;
	for(EdgeIt ie(*gr);ie!=lemon::INVALID;++ie)
	{
		node1=gr->u(ie);
		node2=gr->v(ie);
		protein1=(*node2label)[node1];
		protein2=(*node2label)[node2];
		kst.clear();
		kst.append(protein1);
		kst.append(protein2);
		if(bp12->seWeight.find(kst)==bp12->seWeight.end())
		{
			kst.clear();kst.append(protein2);kst.append(protein1);
		}
    if(!bscore){
			sescore=getScore(bp12->seWeight[kst]);}
	  else{
			sescore=getBitScore(static_cast<float>(bp12->seWeight[kst]));
		}
    stWeight = bp12->stWeight[kst];
    if(isHomoNet){
			weight= (1-alpha)*(sescore-minNodeScore)/serange;
		}
	  else
		{
		 sePart=(1-alpha)*((sescore-minNodeScore)/serange);
		 //ind=static_cast<int>(round((stWeight*1000.0)/maxStrScore));
		 //stPart=alpha*powerlaw[ind];
		 stPart=alpha*pow((stWeight*1.0)/maxStrScore,FACTOR_EDGE);
		 weight = sePart+stPart;
#pragma omp critical
		 {
		 output <<"sequence score\t"<<sescore<<"\t"<<sePart<<"\t structure score\t"<<stWeight/2<<"\t"<<stPart<<std::endl;
		 }
		}
    edgemap->set(ie,weight);
	}
	return true;
}
template<typename KpGraph,typename Option>
bool
RecordStore<KpGraph,Option>::createBpGraph(KpGraph& kpgraph,
                                    NetworkPool& networkpool,
                                    Graph* gr,
                                    OrigLabelNodeMap* node2label,
                                    InvOrigLabelNodeMap* label2node,
                                    EdgeWeight* edgemap,
                                    int ni,
                                    int nj,
                                    int myindex)
{
  BpGraph* bp12=kpgraph.graphs[myindex];
  typename EdgeMap::iterator it;
  typename NetworkPool::GraphData* network_1;
  typename NetworkPool::GraphData* network_2;
  network_1 = networkpool.getGraph(ni);/// The first network
  network_2 = networkpool.getGraph(nj);/// The second network
  std::string protein1,protein2;
  
  for(it=bp12->redBlue.begin();it!=bp12->redBlue.end();++it)
  {
    protein1=it->first;
    protein2=it->second;
    if(!kpgraph.isEdge(protein1,network_1)||!kpgraph.isEdge(protein2,network_2))continue;
    Node node1,node2;
    Edge e;
    std::string kst=protein1;
    kst.append(protein2);
    if(label2node->find(protein1)==label2node->end())
    {
      node1 = gr->addNode();
      node2label->set(node1,protein1);
      (*label2node)[protein1] = node1;
    } 
    else
    {
      node1 = (*label2node)[protein1];
    }
    if(label2node->find(protein2)==label2node->end())
    {
      node2 = gr->addNode();
      node2label->set(node2,protein2);
      (*label2node)[protein2] = node2;
    }
    else
    {
      node2 = (*label2node)[protein2];
    }
    e = gr->addEdge(node1,node2);
  }
  return true;
}

/*template<typename KpGraph,typename Option>
bool
RecordStore<KpGraph,Option>::createRecords_t(KpGraph& kpgraph,NetworkPool& networkpool)
{
  BpGraph* bp12=kpgraph.graphs[0];// Use graphs before running constructGraph.
  typename EdgeMap::iterator it;
  typename NetworkPool::GraphData* network_1;
  typename NetworkPool::GraphData* network_2;
  typename NetworkPool::GraphData* network_k;
  network_1 = networkpool.getGraph(0);// The first network
  network_2 = networkpool.getGraph(1);// The second network
  std::ofstream output(recordfile.c_str());
  for(unsigned i=1;i<numSpecies-1;i++)
  {
    std::string protein1,protein2,protein3,kst1,kst2,kst3;
    double evalue1,evalue2,evalue3;
    network_k = networkpool.getGraph(i+1);// The third network
    BpGraph* bp1k=kpgraph.graphs[i];
    BpGraph* bp2k=kpgraph.graphs[i+numSpecies-2];
    recordarrayarray[i-1] = new RecordArrayValue();
    for(it=bp12->redBlue.begin();it!=bp12->redBlue.end();++it)
    {
      protein1=it->first;protein2=it->second;
      if(!kpgraph.isEdge(protein1,network_1)||!kpgraph.isEdge(protein2,network_2))continue;
      kst1.clear();
      kst1.append(protein1);kst1.append(protein2);
      auto range=bp1k->redBlue.equal_range(protein1);
      typename EdgeMap::iterator pt;
      for(pt=range.first;pt!=range.second;++pt)
      {
        protein3=pt->second;
        if(!kpgraph.isEdge(protein3,network_k))continue;
        kst2.clear();kst3.clear();
        kst2.append(protein1);kst2.append(protein3);
        kst3.append(protein2);kst3.append(protein3);
        evalue1=bp12->seWeight[kst1];evalue2=bp1k->seWeight[kst2];
        if(bp2k->stWeight.find(kst3)!=bp2k->stWeight.end())
        {
          evalue3=bp2k->seWeight[kst3];
          float evalue=scoreCalculate(evalue1,evalue2,evalue3);
          unsigned strWeight=bp12->stWeight[kst1]+bp1k->stWeight[kst2]+bp2k->stWeight[kst3];
          unsigned num=recordarrayarray[i-1]->length++;
          recordarrayarray[i-1]->recordArray[0][num]=protein1;
          recordarrayarray[i-1]->recordArray[1][num]=protein2;
          recordarrayarray[i-1]->recordArray[2][num]=protein3;
          (*recordarrayarray[i-1]->nodeScores)[num]=evalue;
          (*recordarrayarray[i-1]->strScores)[num]=strWeight;
          output << protein1 <<"\t"<< protein2 <<"\t"<< protein3 <<"\t"<<evalue<<"\t"<<strWeight<<std::endl;
        }
      }
    }
  }
  return true;
}*/

/*template<typename KpGraph,typename Option>
bool
RecordStore<KpGraph,Option>::_createRecords(KpGraph& kpgraph,NetworkPool& networkpool)
{
  kpgraph.tCoffee(networkpool);
  readScore(rfile.c_str());
  BpGraph* bp12=kpgraph.graphs[0];
  typename EdgeMap::iterator it;
  typename NetworkPool::GraphData* network_1;
  typename NetworkPool::GraphData* network_2;
  typename NetworkPool::GraphData* network_k;
  network_1 = networkpool.getGraph(0);// The first network
  network_2 = networkpool.getGraph(1);// The second network
  std::ofstream output(recordfile.c_str());
  for(it=bp12->redBlue.begin();it!=bp12->redBlue.end();++it)
  {
    std::string protein1,protein2,protein3,kst1,kst2,kst3;
    double evalue1,evalue2,evalue3;
    protein1=it->first;
    protein2=it->second;
    kst1.clear();
    kst1.append(protein1);kst1.append(protein2);
    if(!kpgraph.isEdge(protein1,network_1)||!kpgraph.isEdge(protein2,network_2))continue;
    for(unsigned i=1;i<NUM_SPECIES-1;++i)
    {
      network_k = networkpool.getGraph(i+1);// The third network
      BpGraph* bp1k=kpgraph.graphs[i];
      BpGraph* bp2k=kpgraph.graphs[i+NUM_SPECIES-2];
      auto range=bp1k->redBlue.equal_range(protein1);
      typename EdgeMap::iterator pt;
      for(pt=range.first;pt!=range.second;++pt)
      {
        protein3=pt->second;
        if(!kpgraph.isEdge(protein3,network_k))continue;
        kst2.clear();kst3.clear();
        kst2.append(protein1);
        kst2.append(protein3);
        kst3.append(protein2);
        kst3.append(protein3);
        evalue1=bp12->seWeight[kst1];
        evalue2=bp1k->seWeight[kst2];
        if(bp2k->stWeight.find(kst3)!=bp2k->stWeight.end())
        {
          // TO DO: output alignment records if number of species is more than 3.
          evalue3=bp2k->seWeight[kst3];//
          float evalue=scoreCalculate(evalue1,evalue2,evalue3);
          unsigned strWeight=bp12->stWeight[kst1]+bp1k->stWeight[kst2]+bp2k->stWeight[kst3];
          output << protein1 <<"\t"<< protein2 <<"\t"<<protein3<<"\t"<< evalue <<"\t"<< strWeight << std::endl;
        }
      }
    }
  }
  return true;
}*/

template<typename KpGraph,typename Option>
bool
RecordStore<KpGraph,Option>::createRecords_MNetAligner(std::vector<std::string>& files,
                                                NetworkPool& networkpool)
{
  const unsigned NUM_FILES=numSpecies*(numSpecies-1)/2;
  MapType* maps = new MapType [NUM_FILES];
  RecordHashMap* lists=new RecordHashMap [NUM_FILES];
  std::ofstream output;
  std::string* record;
  record = new std::string[numSpecies];
  typename NetworkPool::GraphData* network_a;
  typename NetworkPool::GraphData* network_b;
  unsigned indexFile=0;
  for(unsigned i=0;i<numSpecies-1;i++)
  {
    network_a=networkpool.getGraph(i);
    for(unsigned j=i+1;j<numSpecies;j++)
    {
      network_b=networkpool.getGraph(j);
      readHomoList2(maps[indexFile],files[indexFile].c_str(),lists[indexFile],
      network_a->invIdNodeMap,network_b->invIdNodeMap);
      indexFile++;
    }
  }
  readScore(rfile.c_str());
  const MapType& mymap=maps[0];
  if(g_verbosity > VERBOSE_ESSENTIAL)
  {
    output.open(recordfile.c_str());
  }
  for(auto it=mymap.begin();it!=mymap.end();++it)
  {
    std::string firstProtein=it->first;
    record[0]=firstProtein;
    record[1]=it->second.protein;
    getInvolvedRecord(firstProtein,record,1,maps,output,lists);
  }
  delete [] record;
  return true;
}

template<typename KpGraph,typename Option>
bool
RecordStore<KpGraph,Option>::getInvolvedRecord(std::string& firstprotein,
                                        std::string* record,
                                        unsigned j,
                                        MapType* maps,
                                        std::ofstream& output,
                                        RecordHashMap* lists)
{
  if(maps[j].find(firstprotein)==maps[j].end())
    return false;
  auto range = maps[j].equal_range(firstprotein);
  // if firstprotein does not exist, return;
  for(auto it=range.first;it!=range.second;++it)
  {
    record[j+1]=it->second.protein;
    if(beta < 0.999)
    {
      if(distribution(generator)>beta)continue;
    }
    if(j!=numSpecies-2)
    {      
      getInvolvedRecord(firstprotein,record,j+1,maps,output,lists);
    }
    else
    {
      // calculate score of a record;
      float score=0.0;
      if(!_scoreCalculate(record,lists,score))
        continue;
      printRecord(output, record,score);
    }
  }
  return true;
}

template<typename KpGraph,typename Option>
bool
RecordStore<KpGraph,Option>::printRecord(std::ofstream& output,
                                  std::string* record,
                                  float score)
{
  for(unsigned i=0;i<numSpecies;i++)
  {
    output << record[i] <<"\t";
  }
  output << score<<std::endl;
  return true;
}

//template<typename KpGraph,typename Option>
//bool
//RecordStore<KpGraph,Option>::createRecords(const char* file1,
//                                const char* file2,
//                                const char* file3)
//{
//  
//  MapType map1,map2,map3;
//  std::unordered_map<std::string,double> list1,list2,list3;
//  std::string protein1,protein2,protein3;
//  std::ofstream output;
//  double evalue1,evalue2,evalue3;
//  float score;
//  readHomoList(map1,file1,list1);
//  readHomoList(map2,file2,list2);
//  readHomoList(map3,file3,list3);
//  typename MapType::iterator it1,it2;
//  unsigned length=0;
//  readScore(rfile.c_str());
//  if(g_verbosity > VERBOSE_ESSENTIAL)
//  {
//    output.open(recordfile.c_str());
//  }
//  //readDistribution(hfile.c_str(),nfile.c_str());
//  
//  for(it1=map1.begin();it1!=map1.end();++it1)
//  {
//    protein1 = it1->first;
//    protein2 = (it1->second).protein;
//    evalue1 = (it1->second).evalue;
//    auto range = map2.equal_range(protein2);
//    for(it2=range.first;it2!=range.second;++it2)
//    {
//      protein3 = (it2->second).protein;
//      evalue2 = (it2->second).evalue;
//      std::string keystr;
//      keystr.append(protein1);
//      keystr.append(protein3);
//      if(length == MAXIMUM_RECORDS)
//      {
//        std::cerr << "The number of records is in exceese of what expected!\n";
//        return false;
//      }
//      if(list3.find(keystr)!=list3.end())
//      {
//        evalue3 = list3[keystr];
//        (*proteins1)[length]=protein1;
//        (*proteins2)[length]=protein2;
//        (*proteins3)[length]=protein3;
//        score=scoreCalculate(evalue1,evalue2,evalue3);
//        (*nodeScores)[length]=score;
//        if(g_verbosity > VERBOSE_ESSENTIAL)
//        {
//          output <<protein1 <<"\t" << protein2 <<"\t"<<protein3<<"\t"<<score;
//          output << std::endl;
//        }
//        length++;
//      }
//    }
//  }
//  if(g_verbosity > VERBOSE_ESSENTIAL)
//    output.close();
//  return true;
//}

template<typename KpGraph,typename Option>
bool
RecordStore<KpGraph,Option>::readHomoList(MapType& mymap,
                               const char* filename,
                               std::unordered_map<std::string,double>& homologylist)
{
  std::ifstream input(filename);
  std::string line;
  std::string protein1="";
  std::string protein2="";
  int num2,num3,num4,num5,num6,num7,num8;
  float num1,bitscore;
  double evalue;
  if(!input.is_open())
  {
    std::cerr << filename << "cannot be opened" <<std::endl;
    return false;
  }
  while(std::getline(input,line))
  {
    std::stringstream lineStream(line);
    lineStream >> protein1 >> protein2 >> num1 >> num2 >> num3 >> num4
    >> num5 >> num6 >> num7 >> num8 >> evalue >> bitscore;
    std::string keystring;
    keystring.append(protein1);
    keystring.append(protein2);
    // To do(Jialu): parse the records have not responding proteins in networks.
    if(homologylist.find(keystring)!=homologylist.end())
    {
      if(homologylist[keystring] > evalue)
      {
        // in case repeated records appeaed in blast E-Value files.
        homologylist[keystring] = evalue;
        mymap.insert(std::pair<std::string,ValueType>(protein1,ValueType(protein2,evalue)));
      }
      else
      {
        continue;
      }
    }
    else
    {
      homologylist[keystring] = evalue;
      mymap.insert(std::pair<std::string,ValueType>(protein1, ValueType(protein2,evalue)));
    }
  }
  return true;
}

template<typename KpGraph,typename Option>
bool
RecordStore<KpGraph,Option>::readHomoList2(MapType& mymap,
                               const char* filename,
                               std::unordered_map<std::string,double>& homologylist,
                               InvOrigLabelNodeMap* inv1,
                               InvOrigLabelNodeMap* inv2)
                               /// Check validation when create records.
{
  std::ifstream input(filename);
  std::string line;
  std::string protein1="";
  std::string protein2="";

  double evalue;
  if(!input.is_open())
  {
    std::cerr << filename << "cannot be opened!" <<std::endl;
    return false;
  }
  while(std::getline(input,line))
  {
    std::stringstream lineStream(line);
    lineStream >> protein1 >> protein2 >> evalue;
    /// Check the validation of protein1 and protein2. Isf they are not in networks, then ignore.
    if(inv1->find(protein1)==inv1->end() ||
       inv2->find(protein2)==inv2->end()
       ) continue;
    std::string keystring;
    keystring.append(protein1);
    keystring.append(protein2);
    if(homologylist.find(keystring)!=homologylist.end())
    {
      if(homologylist[keystring] > evalue)
      {
        // in case repeated records appeaed in blast E-Value files.
        homologylist[keystring] = evalue;
        mymap.insert(std::pair<std::string,ValueType>(protein1,ValueType(protein2,evalue)));
      }
      else
      {
        continue;
      }
    }
    else
    {
      homologylist[keystring] = evalue;
      mymap.insert(std::pair<std::string,ValueType>(protein1, ValueType(protein2,evalue)));
    }
  }
  return true;
}

template<typename KpGraph,typename Option>
bool
RecordStore<KpGraph,Option>::readDistribution(const char * filename1, const char* filename2)
{
  std::ifstream input1(filename1);
  std::ifstream input2(filename2);
  std::string line;
  double region;
  float probability;
  unsigned linenum=0;

  if(!input1.is_open() || !input2.is_open())
  {
    std::cerr <<"Training data is not available\n";
    return false;
  }

  while(std::getline(input1,line))
  {
    std::stringstream streamline(line);
    streamline >> region >> probability;
    homoDistr[linenum]=probability;
    distriBar[linenum]=region;
    linenum++;
  }
  
  linenum=0;
  while(std::getline(input2,line))
  {
    std::stringstream streamline(line);
    streamline >> region >> probability;
    nullDistr[linenum]=probability;
    linenum++;
  }
  input1.close();
  input2.close();
  return true;
}

template<typename KpGraph,typename Option>
bool
RecordStore<KpGraph,Option>::readScore(const char * filename)
{
  std::ifstream input(filename);
  std::string line;
  float probability;
  int linenum=0;
  
  if(!input.is_open())
  {
    std::cerr <<"Score file is not available\n";
    return false;
  }

  if(g_verbosity > VERBOSE_ESSENTIAL)
  {
    //std::cerr <<"------Scoring function of E-Value!-------"<<std::endl;
  }
  bool first_line=true;
  while(std::getline(input,line))
  {
    std::stringstream streamline(line);
    streamline >> probability;
    if((linenum<88 && !bscore) || (linenum>1 && bscore) )
    {
	  if(first_line)
	  {
	      maxNodeScore = probability;
	      minNodeScore = probability;
	      first_line = false;
      }
      if (probability > maxNodeScore)
        maxNodeScore = probability;
      if (probability < minNodeScore)
        minNodeScore = probability;
    }
    (*resultDistr)[linenum]=probability;
    if(g_verbosity > VERBOSE_ESSENTIAL)
    {
      //std::cout << probability << std::endl;
    }
    linenum++;
  }
  input.close();
  return true;
}

template<typename KpGraph,typename Option>
float
RecordStore<KpGraph,Option>::scoreCalculate(double e1,double e2,double e3)
{
  float score;
  score=getScore(e1)+getScore(e2)+getScore(e3);
  return score;
}

template<typename KpGraph,typename Option>
bool
RecordStore<KpGraph,Option>::_scoreCalculate(std::string* record,
                                      RecordHashMap* lists,float& score)
{
  unsigned myindex=0;
  for(unsigned i=0;i<numSpecies-1;i++)
  {
    std::string firstprotein=record[i];
    for(unsigned j=i+1;j<numSpecies;j++)
    {
      std::string keyprotein(firstprotein);
      keyprotein.append(record[j]);
      if(lists[myindex].find(keyprotein)==lists[myindex].end())
        return false;
      score += getScore((lists[myindex++])[keyprotein]);
    }
  }
  return true;
}

template<typename KpGraph,typename Option>
float
RecordStore<KpGraph,Option>::getScore(double evalue)
{
  int index=0;
  if(evalue>1.0e-180)
  {
    index = static_cast<int>(ceil(log10(evalue))/NUM_PRO_INTERVAL)+NUM_OFFSET_BARS;
    //std::cout << evalue << index <<std::endl;
  }
  if(index>=NUM_PRO_BARS)
  {
    std::cerr <<"E-Value "<<evalue<<" is out of range!"<<std::endl;
    return 0.0;
  }
  return (*resultDistr)[index];
}

template<typename KpGraph,typename Option>
float
RecordStore<KpGraph,Option>::getBitScore(float bitscore)
{
	int ind=0;
	if(bitscore < 940.0)
	{
		ind=bitscore/10;
	}else
	{
		ind=NUM_PRO_BARS-1;
	}
	return (*resultDistr)[ind];
}

template<typename KpGraph,typename Option>
float
RecordStore<KpGraph,Option>::combineScore(float seScore, unsigned stScore)
{
  std::string filename(resultfolder);
  filename.append("scoreRecords.txt");
  float sePart,stPart;
  sePart=(1-alpha)*((seScore-minNodeScore)/(maxNodeScore-minNodeScore));
	int ind=static_cast<int>(round((stScore*1000.0)/maxStrScore));
	stPart=alpha*powerlaw[ind];
	//std::ofstream output(filename.c_str(),std::ios_base::out|std::ios_base::app);
 // output <<"sequence score\t"<<seScore<<"\t"<<sePart<<"\t structure score\t"<<stScore/2<<"\t"<<stPart<<std::endl;
 // output.close();
  return sePart+stPart;
}

template<typename KpGraph,typename Option>
void
RecordStore<KpGraph,Option>::top10000(std::string& filename)
{
	typedef struct _ElementType
	{
		std::string protein1;
		std::string protein2;
		float score;
	}ElementType;
	struct ElementCompare{
		bool operator() (const ElementType& e1,const ElementType& e2) const
		{
			return e1.score >= e2.score;
		}
	}desceasing;
	std::vector<ElementType> homoLogSet,homoLogRatioSet;
	std::ifstream input(filename.c_str());
	std::ofstream output1("./result/investigation/toplog/toplog.txt");
	std::ofstream output2("./result/investigation/toplogratio/toplogratio.txt");
	if(!input.is_open()){
		std::cerr <<"Homology file "<<filename<< " doesn't exist!"<<std::endl;
		return;
	}
	std::string line;
	std::unordered_map<std::string,int> homomap;
	readScore(rfile.c_str());
	while(std::getline(input,line))
	{
		std::string protein1,protein2;
		float bitscore;
		double evalue;
		std::stringstream lineStream(line);
		lineStream >> protein1 >> protein2 >> bitscore >> evalue;
		std::string keystring;
		if(protein1.compare(protein2)==0)
			continue;
		if(protein1.compare(protein2)<0)
		{
			keystring.append(protein1);
			keystring.append(protein2);
		}else
		{
			keystring.append(protein2);
			keystring.append(protein1);
		}
		
		if(homomap.find(keystring)!=homomap.end())continue;
		homomap[keystring]=1;
		ElementType e1,e2;
		e1.protein1=e2.protein1=protein1;
		e1.protein2=e2.protein2=protein2;
		if(evalue<1e-180)
			e1.score=190;
		else
			e1.score=-log10(evalue);
		int linenum=getIndex(evalue);
		e2.score=(*resultDistr)[linenum];
		homoLogSet.push_back(e1);
		homoLogRatioSet.push_back(e2);
	}
	std::stable_sort(homoLogSet.begin(),homoLogSet.end(),desceasing);
	std::stable_sort(homoLogRatioSet.begin(), homoLogRatioSet.end(), desceasing);
	for(int i=0;i<10000;i++)
	{
		output1 << homoLogSet[i].protein1 <<"\t" << homoLogSet[i].protein2 << std::endl;
		output2 << homoLogRatioSet[i].protein1 <<"\t"<<homoLogRatioSet[i].protein2<<std::endl;
	}
	output1.close();
	output2.close();
}
#endif //RECORDSTORE
