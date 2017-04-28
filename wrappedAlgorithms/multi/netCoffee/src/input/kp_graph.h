/* kp_graph.h
Author: Jialu Hu
Date: 28.06.2012*/

#ifndef KP_GRAPH_H_
#define KP_GRAPH_H_

#include <lemon/core.h>
#include <lemon/bits/graph_extender.h>
#include <iostream>
#include <string>
#include <array>
#include <unordered_map>
#include <utility>
#include "macro.h"
#include "verbose.h"
#include <assert.h>

template<typename NetworkPool>
class KpGraph
{
public:
  typedef NetworkPool NetwookPool_Type;
  typedef std::unordered_multimap<std::string,std::string> EdgeMap;
  typedef std::unordered_map<std::string,double> WeightMap;
  typedef std::unordered_map<std::string,unsigned> StWeightMap;
  typedef typename NetworkPool::Graph Graph;
  typedef typename NetworkPool::GraphData GraphData;
  typedef typename NetworkPool::OrigLabelNodeMap OrigLabelNodeMap;
  typedef typename NetworkPool::InvOrigLabelNodeMap InvOrigLabelNodeMap;
  typedef typename Graph::template EdgeMap<float> EdgeWeight;
  typedef struct _BpGraph
  {
    EdgeMap redBlue;
    //EdgeMap blueRed;
    WeightMap seWeight;
    StWeightMap stWeight;
  }BpGraph;
  TEMPLATE_GRAPH_TYPEDEFS(Graph);
	typedef struct _PrivateVariable
	{
		typename NetworkPool::GraphData *network_1,*network_2,*network_k;
		BpGraph *bp12,*bp1k,*bp2k;
		EdgeMap::iterator it,pt;
		IncEdgeIt ie,pe,me;
		int nc,x;
		std::string protein1,protein2,protein3,protein4,protein5,protein6,kst1,kst2,kst3,kst4,kst5,kst6;//Protein in left and right circles.
		Node nodeA1,nodeA2,nodeA3,nodeB1,nodeB2,nodeB3;
		std::pair<EdgeMap::iterator,EdgeMap::iterator> range;
		_PrivateVariable(){nc=0;x=0;}
	}PrivateVariable;
  
  std::vector<std::string> fileList;
  std::vector<BpGraph*> graphs;
  int numSpecies;
  unsigned maxStrWeight;
  
  KpGraph(std::vector<std::string>&,int);//{fileList=files;}
  int  getBpIndex(int,int);
  bool readHomoList(std::string&,BpGraph*,int,int);
  bool constructGraph(int);
  bool reweighting_parallel(NetworkPool&,int,int,PrivateVariable&);
	bool outputWeight(NetworkPool&,int,int,PrivateVariable&);
  bool reweighting(NetworkPool&,int,int);
  bool reweightingAll(NetworkPool&,int);
  bool isEdge(std::string protein,GraphData*);
  bool createBpGraph(Graph&,OrigLabelNodeMap&,InvOrigLabelNodeMap&,EdgeWeight&,NetworkPool&);
};

template<typename NetworkPool>
KpGraph<NetworkPool>::KpGraph(std::vector<std::string>& files,int species)
:numSpecies(species)
,maxStrWeight(0)
{
  fileList=files;
}
template<typename NetworkPool>
bool KpGraph<NetworkPool>::isEdge(std::string protein,GraphData* network)
{
  if( network->invIdNodeMap->find(protein)==network->invIdNodeMap->end())
  {
    return false;
  }
  return true;
}
template<typename NetworkPool>
bool KpGraph<NetworkPool>::createBpGraph(Graph& gr,
                                         OrigLabelNodeMap& node2label,
                                         InvOrigLabelNodeMap& label2node,
                                         EdgeWeight& edgemap,
                                         NetworkPool& networkpool)
{
  BpGraph* bp12=graphs[0];
  EdgeMap::iterator it;
  typename NetworkPool::GraphData* network_1;
  typename NetworkPool::GraphData* network_2;
  network_1 = networkpool.getGraph(0);// The first network
  network_2 = networkpool.getGraph(1);// The second network
  std::string protein1,protein2;
  for(it=bp12->redBlue.begin();it!=bp12->redBlue.end();++it)
  {
    protein1=it->first;
    protein2=it->second;
    if(!isEdge(protein1,network_1)||!isEdge(protein2,network_2))continue;
    Node node1,node2;
    Edge e;
    if(label2node.find(protein1)==label2node.end())
    {
      node1 = gr.addNode();
      node2label.set(node1,protein1);
      label2node[protein1] = node1;
    } 
    else
    {
      node1 = label2node[protein1];
    }
    if(label2node.find(protein2)==label2node.end())
    {
      node2 = gr.addNode();
      node2label.set(node2,protein2);
      label2node[protein2] = node2;
    }
    else
    {
      node2 = label2node[protein2];
    }
    e = gr.addEdge(node1,node2);
    edgemap.set(e,0.0);
  }
  return true;
}

template<typename NetworkPool>
int KpGraph<NetworkPool>::getBpIndex(int i,int j)
{
  if (i>=j || j>=numSpecies)
    std::cerr << "Triplets doesn't exist!" << std::endl;
    return (i*(2*numSpecies-i-1))/2+j;
}

template<typename NetworkPool>
bool KpGraph<NetworkPool>::constructGraph(int numthreads)
{
  graphs.clear();
  int ni,nj;
	ni=0;nj=-1;
	int numBp=numSpecies*(numSpecies+1)/2;
	for(int i=0;i<numBp;i++)
	{
		int lni,lnj;
		#pragma omp critical
		{
			if(nj<numSpecies-1)nj++;
			else{ni++;nj=ni;}
			lni=ni;
			lnj=nj;
			graphs.push_back(new BpGraph());
		}
		readHomoList(fileList[i],graphs[i],lni,lnj);
	}
  return true;
}

//template<typename NetworkPool>
//float KpGraph<NetworkPool>::scoreCalculate(double e1,double e2,double e3)
//{
  //float score;
  //score=getScore(e1)+getScore(e2)+getScore(e3);
  //return score;
//}

//template<typename NetworkPool>
//float KpGraph<NetworkPool>::getScore(double evalue)
//{
  //int index=0;
  //if(evalue>1.0e-180)
  //{
    //index = static_cast<int>(ceil(log10(evalue))/NUM_PRO_INTERVAL)+NUM_OFFSET_BARS;
    ////std::cout << evalue << index <<std::endl;
  //}
  //if(index>=NUM_PRO_BARS)
  //{
    //std::cerr <<"E-Value "<<evalue<<" is out of range!"<<std::endl;
    //return 0.0;
  //}
  //return (*resultDistr)[index];
//}

template<typename NetworkPool>
bool KpGraph<NetworkPool>::readHomoList(std::string& filename,BpGraph* graph,int ni,int nj)
{
	std::ifstream input(filename.c_str(),std::ios_base::in);
  std::string line;
  std::string protein1="";
  std::string protein2="";
  double evalue;
  if(!input.is_open())
  {
    std::cerr << filename << "cannot be opened" <<std::endl;
    return false;
  }
  while(std::getline(input,line))
  {
    std::stringstream lineStream(line);
    lineStream >> protein1 >> protein2 >> evalue;
    std::string keystring;
    if(ni==nj)
    {
			int com=protein1.compare(protein2);
		if(com>0)
		{
			std::string temp = protein1;
			protein1 = protein2;
			protein2 = temp;
		}else if(com==0)continue;
		}
	keystring.append(protein1);keystring.append(protein2);
    if(graph->seWeight.find(keystring)!=graph->seWeight.end())
    {
      if(graph->seWeight[keystring]>evalue)
        graph->seWeight[keystring]=evalue;
    }
    else
    {
      graph->seWeight[keystring]=evalue;
      graph->stWeight[keystring]=0;
      graph->redBlue.insert(std::make_pair(protein1,protein2));
      //graph->blueRed.insert(std::make_pair(protein2,protein1));
    }
  }
	std::cout << ni <<"\t" << nj <<"\t" << graph->seWeight.size()<<std::endl;
	input.close();
  return true;
}

template<typename NetworkPool>
bool KpGraph<NetworkPool>::reweightingAll(NetworkPool& networkpool,int numthreads)
{
  constructGraph(numthreads);
	int ni,nj;
	ni=nj=0;

	PrivateVariable myPrivateVariable;
#pragma omp parallel for num_threads(numthreads) shared(ni,nj,networkpool) schedule(dynamic,1) private(myPrivateVariable)
	for(int num=0;num<numSpecies*(numSpecies-1)/2;num++)
	{
		int lni,lnj;
		#pragma omp critical
		{
			if(nj<numSpecies-1)nj++;
			else{ni++;nj=ni+1;}
			lni=ni;lnj=nj;
		}
		reweighting_parallel(networkpool,lni,lnj,myPrivateVariable);
	}	

	//#pragma omp parallel for num_threads(numthreads) shared(ni,nj,networkpool) schedule(dynamic,1) private(myPrivateVariable)
	/*ni=nj=0;
	for(int num=0;num<numSpecies*(numSpecies-1)/2;num++)
	{
			if(nj<numSpecies-1)nj++;
			else{ni++;nj=ni+1;}
		outputWeight(networkpool,ni,nj,myPrivateVariable);
	}	*/
	//#pragma omp parallel for collapse(2) for() for();; omp_set_nested(1);
  return true;
}

template<typename NetworkPool>
bool KpGraph<NetworkPool>::outputWeight(NetworkPool& networkpool,int ni,int nj,PrivateVariable& myPrivateVariable)
{
	myPrivateVariable.network_1 = networkpool.getGraph(ni);// The first network
	myPrivateVariable.network_2 = networkpool.getGraph(nj);// The second network
	myPrivateVariable.bp12=graphs[getBpIndex(ni,nj)];
	
	for(myPrivateVariable.it=myPrivateVariable.bp12->redBlue.begin();myPrivateVariable.it!=myPrivateVariable.bp12->redBlue.end();++myPrivateVariable.it)
	{
			myPrivateVariable.protein1=myPrivateVariable.it->first;
	    myPrivateVariable.protein2=myPrivateVariable.it->second;
			if(!isEdge(myPrivateVariable.protein1,myPrivateVariable.network_1)||!isEdge(myPrivateVariable.protein2,myPrivateVariable.network_2))continue;
	    myPrivateVariable.kst1.append(myPrivateVariable.protein1);myPrivateVariable.kst1.append(myPrivateVariable.protein2);
				std::cout << myPrivateVariable.protein1 << "\t" << myPrivateVariable.protein2 <<"\t" <<myPrivateVariable.bp12->stWeight[myPrivateVariable.kst1] << std::endl;
				myPrivateVariable.kst1.clear();
	}
	return true;
}

template<typename NetworkPool>
bool KpGraph<NetworkPool>::reweighting_parallel(NetworkPool& networkpool,int ni,int nj,PrivateVariable& myPrivateVariable)
{
	myPrivateVariable.network_1 = networkpool.getGraph(ni);// The first network
	myPrivateVariable.network_2 = networkpool.getGraph(nj);// The second network
	myPrivateVariable.bp12=graphs[getBpIndex(ni,nj)];
	for(myPrivateVariable.it=myPrivateVariable.bp12->redBlue.begin();myPrivateVariable.it!=myPrivateVariable.bp12->redBlue.end();++myPrivateVariable.it,++myPrivateVariable.nc)
	{
	    myPrivateVariable.protein1=myPrivateVariable.it->first;
	    myPrivateVariable.protein2=myPrivateVariable.it->second;
	    if(!isEdge(myPrivateVariable.protein1,myPrivateVariable.network_1)||!isEdge(myPrivateVariable.protein2,myPrivateVariable.network_2))continue;
	    myPrivateVariable.kst1.append(myPrivateVariable.protein1);myPrivateVariable.kst1.append(myPrivateVariable.protein2);
	    myPrivateVariable.nodeA1=(*myPrivateVariable.network_1->invIdNodeMap)[myPrivateVariable.protein1];
	    myPrivateVariable.nodeA2=(*myPrivateVariable.network_2->invIdNodeMap)[myPrivateVariable.protein2];
		for(myPrivateVariable.x=nj+1;myPrivateVariable.x<numSpecies;++myPrivateVariable.x)
	    {
			myPrivateVariable.network_k = networkpool.getGraph(myPrivateVariable.x);// The third network
			myPrivateVariable.bp1k=graphs[getBpIndex(ni,myPrivateVariable.x)];
			myPrivateVariable.bp2k=graphs[getBpIndex(nj,myPrivateVariable.x)];
			myPrivateVariable.range=myPrivateVariable.bp1k->redBlue.equal_range(myPrivateVariable.protein1);
			for(myPrivateVariable.pt=myPrivateVariable.range.first;myPrivateVariable.pt!=myPrivateVariable.range.second;++myPrivateVariable.pt)
		    {
		        myPrivateVariable.protein3=myPrivateVariable.pt->second;
		        if(!isEdge(myPrivateVariable.protein3,myPrivateVariable.network_k))continue;
		        myPrivateVariable.nodeA3 =(*myPrivateVariable.network_k->invIdNodeMap)[myPrivateVariable.protein3];
		        myPrivateVariable.kst2.append(myPrivateVariable.protein1);
		        myPrivateVariable.kst2.append(myPrivateVariable.protein3);
		        myPrivateVariable.kst3.append(myPrivateVariable.protein2);
		        myPrivateVariable.kst3.append(myPrivateVariable.protein3);
		        if(myPrivateVariable.bp2k->stWeight.find(myPrivateVariable.kst3)!=myPrivateVariable.bp2k->stWeight.end())
		        {
		          /// Find a left circle. Then we need to check whether there exist a right circle.
		          for(myPrivateVariable.ie=IncEdgeIt(*(myPrivateVariable.network_1->g),myPrivateVariable.nodeA1);myPrivateVariable.ie!=lemon::INVALID;++myPrivateVariable.ie)
		          {
					  myPrivateVariable.nodeB1=myPrivateVariable.network_1->g->runningNode(myPrivateVariable.ie);
			            myPrivateVariable.protein4=(*myPrivateVariable.network_1->label)[myPrivateVariable.nodeB1];
			            if(!isEdge(myPrivateVariable.protein4,myPrivateVariable.network_1))continue;
			            for(myPrivateVariable.pe=IncEdgeIt((*myPrivateVariable.network_2->g),myPrivateVariable.nodeA2);myPrivateVariable.pe!=lemon::INVALID;++myPrivateVariable.pe)
			            {
			              myPrivateVariable.nodeB2=myPrivateVariable.network_2->g->runningNode(myPrivateVariable.pe);
			              myPrivateVariable.protein5=(*myPrivateVariable.network_2->label)[myPrivateVariable.nodeB2];
			              if(!isEdge(myPrivateVariable.protein5,myPrivateVariable.network_2))continue;
			              myPrivateVariable.kst4.append(myPrivateVariable.protein4);
			              myPrivateVariable.kst4.append(myPrivateVariable.protein5);
										if(myPrivateVariable.bp12->stWeight.find(myPrivateVariable.kst4)==myPrivateVariable.bp12->stWeight.end()){myPrivateVariable.kst4.clear();continue;}
						  for(myPrivateVariable.me=IncEdgeIt((*myPrivateVariable.network_k->g),myPrivateVariable.nodeA3);myPrivateVariable.me!=lemon::INVALID;++myPrivateVariable.me)
			              {
			                myPrivateVariable.nodeB3=myPrivateVariable.network_k->g->runningNode(myPrivateVariable.me);
			                myPrivateVariable.protein6=(*myPrivateVariable.network_k->label)[myPrivateVariable.nodeB3];
			                if(!isEdge(myPrivateVariable.protein6,myPrivateVariable.network_k))continue;
			                myPrivateVariable.kst5.append(myPrivateVariable.protein4);
			                myPrivateVariable.kst5.append(myPrivateVariable.protein6);
			                myPrivateVariable.kst6.append(myPrivateVariable.protein5);
			                myPrivateVariable.kst6.append(myPrivateVariable.protein6);
			                if(myPrivateVariable.bp1k->stWeight.find(myPrivateVariable.kst5)!=myPrivateVariable.bp1k->stWeight.end()
			                  && myPrivateVariable.bp2k->stWeight.find(myPrivateVariable.kst6)!=myPrivateVariable.bp2k->stWeight.end())
			                  {
			                    /// reweight on match edges.
#pragma omp critical
													{
			                    myPrivateVariable.bp1k->stWeight[myPrivateVariable.kst2]++;
			                    myPrivateVariable.bp1k->stWeight[myPrivateVariable.kst5]++;
			                    myPrivateVariable.bp2k->stWeight[myPrivateVariable.kst3]++;
			                    myPrivateVariable.bp2k->stWeight[myPrivateVariable.kst6]++;
			                    myPrivateVariable.bp12->stWeight[myPrivateVariable.kst1]++;
													myPrivateVariable.bp12->stWeight[myPrivateVariable.kst4]++;
			                    if(maxStrWeight < myPrivateVariable.bp12->stWeight[myPrivateVariable.kst1])
			                    {
			                      maxStrWeight = myPrivateVariable.bp12->stWeight[myPrivateVariable.kst1];
													}
													if(maxStrWeight < myPrivateVariable.bp12->stWeight[myPrivateVariable.kst4])
			                    {
			                      maxStrWeight = myPrivateVariable.bp12->stWeight[myPrivateVariable.kst4];
													}
													if(maxStrWeight < myPrivateVariable.bp1k->stWeight[myPrivateVariable.kst2])
			                    {
			                      maxStrWeight = myPrivateVariable.bp1k->stWeight[myPrivateVariable.kst2];
													}
													if(maxStrWeight < myPrivateVariable.bp1k->stWeight[myPrivateVariable.kst5])
			                    {
			                      maxStrWeight = myPrivateVariable.bp1k->stWeight[myPrivateVariable.kst5];
													}
													if(maxStrWeight < myPrivateVariable.bp2k->stWeight[myPrivateVariable.kst3])
			                    {
			                      maxStrWeight = myPrivateVariable.bp2k->stWeight[myPrivateVariable.kst3];
													}
													if(maxStrWeight < myPrivateVariable.bp2k->stWeight[myPrivateVariable.kst6])
			                    {
			                      maxStrWeight = myPrivateVariable.bp2k->stWeight[myPrivateVariable.kst6];
													}
													}
			                  }
			                  myPrivateVariable.kst5.clear();myPrivateVariable.kst6.clear();
						  }
						  myPrivateVariable.kst4.clear();
						}
				  }
				}
				 myPrivateVariable.kst2.clear();myPrivateVariable.kst3.clear();
			}
		}
		myPrivateVariable.kst1.clear();
	}
	return true;
}
template<typename NetworkPool>
bool KpGraph<NetworkPool>::reweighting(NetworkPool& networkpool,int ni,int nj)
{
  int myindex=0;
  myindex=getBpIndex(ni,nj);
  EdgeMap::iterator it;
  typename NetworkPool::GraphData* network_1;
  typename NetworkPool::GraphData* network_2;
  typename NetworkPool::GraphData* network_k;
  network_1 = networkpool.getGraph(ni);// The first network
  network_2 = networkpool.getGraph(nj);// The second network
  BpGraph* bp12=graphs[myindex];
  Node nodeA1,nodeA2,nodeA3,nodeB1,nodeB2,nodeB3;
  std::string kst1,kst2,kst3,kst4,kst5,kst6;
  int nc=0;
  for(it=bp12->redBlue.begin();it!=bp12->redBlue.end();++it,++nc)
  {
    //std::cerr <<"The number of steps: "<<nc<<std::endl;
    std::string protein1,protein2,protein3,protein4,protein5,protein6;//Protein in left and right circles.
    protein1=it->first;
    protein2=it->second;
    if(!isEdge(protein1,network_1)||!isEdge(protein2,network_2))continue;
    kst1.append(protein1);kst1.append(protein2);
    nodeA1=(*network_1->invIdNodeMap)[protein1];
    nodeA2=(*network_2->invIdNodeMap)[protein2];
    for(int x=nj+1;x<numSpecies;++x)
    {
      network_k = networkpool.getGraph(x);// The third network
      BpGraph* bp1k=graphs[getBpIndex(ni,x)];
      BpGraph* bp2k=graphs[getBpIndex(nj,x)];
      auto range=bp1k->redBlue.equal_range(protein1);
      EdgeMap::iterator pt;
      for(pt=range.first;pt!=range.second;++pt)
      {
        protein3=pt->second;
        if(!isEdge(protein3,network_k))continue;
        nodeA3 =(*network_k->invIdNodeMap)[protein3];
        kst2.append(protein1);
        kst2.append(protein3);
        kst3.append(protein2);
        kst3.append(protein3);
        if(bp2k->stWeight.find(kst3)!=bp2k->stWeight.end())
        {
          /// Find a left circle. Then we need to check whether there exist a right circle.
          for(IncEdgeIt ie((*network_1->g),nodeA1);ie!=lemon::INVALID;++ie)
          {
            nodeB1=network_1->g->runningNode(ie);
            protein4=(*network_1->label)[nodeB1];
            if(!isEdge(protein4,network_1))continue;
            for(IncEdgeIt pe((*network_2->g),nodeA2);pe!=lemon::INVALID;++pe)
            {
              nodeB2=network_2->g->runningNode(pe);
              protein5=(*network_2->label)[nodeB2];
              if(!isEdge(protein5,network_2))continue;
              kst4.append(protein4);
              kst4.append(protein5);
              for(IncEdgeIt me((*network_k->g),nodeA3);me!=lemon::INVALID;++me)
              {
                nodeB3=network_k->g->runningNode(me);
                protein6=(*network_k->label)[nodeB3];
                if(!isEdge(protein6,network_k))continue;
                kst5.append(protein4);
                kst5.append(protein6);
                kst6.append(protein5);
                kst6.append(protein6);
                if(bp1k->stWeight.find(kst5)!=bp1k->stWeight.end()
                  && bp2k->stWeight.find(kst6)!=bp2k->stWeight.end())
                  {
                    /// reweight on match edges.
                    bp1k->stWeight[kst2]++;
                    bp1k->stWeight[kst5]++;
                    bp2k->stWeight[kst3]++;
                    bp2k->stWeight[kst6]++;
                    bp12->stWeight[kst1]++;
                    if(maxStrWeight < bp12->stWeight[kst1])
                      maxStrWeight = bp12->stWeight[kst1];
                    if(bp12->stWeight.find(kst4)!=bp12->stWeight.end())
                    {
                      bp12->stWeight[kst4]++;
                      if(maxStrWeight < bp12->stWeight[kst4])
                        maxStrWeight = bp12->stWeight[kst4];
                    }
                  }
                  kst5.clear();kst6.clear();
              }
              kst4.clear();
            }
          }
        }
        kst2.clear();kst3.clear();
      }
    }
    kst1.clear();
  }
  return true;
}
#endif /// KP_GRAPH_H_
