/* networkpool.h
Author: Jialu Hu
Date: 18.06.2012*/

#ifndef NETWORKPOOL_H_
#define NETWORKPOOL_H_

#include <vector>
#include <lemon/core.h>
#include <lemon/bits/graph_extender.h>
#include <unordered_map>
#include "verbose.h"
#include <assert.h>
using namespace lemon;

template<typename GR, typename BP>
class NetworkPool
{
public:
  typedef GR Graph;
  typedef BP BpGraph;
public:
  TEMPLATE_GRAPH_TYPEDEFS(Graph);

  /// Labels of the nodes
  typedef typename Graph::template NodeMap<std::string> OrigLabelNodeMap;
  /// Mapping from labels to original nodes
  typedef std::unordered_map<std::string, typename Graph::Node> InvOrigLabelNodeMap;
  /// Weights on original edges
  typedef typename Graph::template EdgeMap<int> WeightEdgeMap;
  /// Host of proteins for all input networks
  std::unordered_map<std::string,short> proteinHost;
  typedef typename Graph::template NodeMap<unsigned> DegreeNodeMap;
  int allNodeNum;

  struct GraphData
  {
    Graph *g;
    OrigLabelNodeMap *label;
    InvOrigLabelNodeMap *invIdNodeMap;
    DegreeNodeMap *degreeMap;
    WeightEdgeMap *weight;
    lemon::ArcLookUp<Graph> *arcLookUpG;
    int nodeNum;
    int edgeNum;
    GraphData()
    {
      nodeNum=0;
      edgeNum=0;
      g = new Graph();
      label = new OrigLabelNodeMap(*g);
      invIdNodeMap = new InvOrigLabelNodeMap();
      degreeMap = new DegreeNodeMap(*g);
      weight = new WeightEdgeMap(*g);
      arcLookUpG = new lemon::ArcLookUp<Graph>(*g);
    }
    ~GraphData()
    {
      delete g;
      delete label;
      delete invIdNodeMap;
      delete degreeMap;
      delete weight;
      delete arcLookUpG;
    }
  };
  std::vector<GraphData*> _graphSet;
  
	NetworkPool();
	~NetworkPool();
	bool initNetworkPool(std::vector<std::string>&,int);
	bool readNetwork(std::string&,int);
  GraphData* getGraph(int);
  unsigned getHost(std::string);
  bool existNode(std::string);
};

template<typename GR, typename BP>
NetworkPool<GR,BP>::NetworkPool()
:allNodeNum(0)
{
}

template<typename GR, typename BP>
NetworkPool<GR,BP>::~NetworkPool()
{
  for(unsigned i=0;i<_graphSet.size();i++)
    delete _graphSet[i];
}

template<typename GR, typename BP>
typename NetworkPool<GR,BP>::GraphData* NetworkPool<GR,BP>::getGraph(int i)
{
  //std::cout << _graphSet.size() << std::endl;
  return _graphSet[i];
}

template<typename GR, typename BP>
unsigned NetworkPool<GR,BP>::getHost(std::string protein)
{
  if(proteinHost.find(protein)==proteinHost.end()) return 100;// 100 represents protein doesn't exist in input networks.
  return proteinHost[protein];
}

template<typename GR, typename BP>
bool NetworkPool<GR,BP>::existNode(std::string protein)
{
  if(proteinHost.find(protein)!=proteinHost.end())
  return true;
  else
  return false;
}

template<typename GR, typename BP>
bool NetworkPool<GR,BP>::initNetworkPool(std::vector<std::string> &filelist,int numthreads)
{
  std::vector<std::string>::iterator it;
  int i,fsize;
	fsize=filelist.size();
//#pragma omp parallel for num_threads(numthreads) ordered
  for(i=0;i<fsize;++i)
	{
    readNetwork(filelist[i],i);
  }
  return 1;
}

template<typename GR, typename BP>
bool NetworkPool<GR,BP>::readNetwork(std::string &filename,int i)
{
  GraphData *data = new GraphData();// private
  std::string line;
  std::unordered_map<std::string,int> interactionmap;
  std::ifstream input(filename.c_str());
  if(!input.good())
  {
    std::cerr << filename <<"cannot be opened!"<<std::endl;
    return 0;
  }
#pragma omp ordered
	{
	_graphSet.push_back(data);
	}
  //_graphSet[i]=data;// work sharing
  std::getline(input,line);/// Skip header line: INTERACTOR A INTERACTOR B
  while(std::getline(input,line))
  {
    std::string protein1, protein2, tempstr, keystr;
    std::stringstream lineStream(line);
    lineStream >> protein1 >> protein2;
    if(protein1.compare(protein2)==0)
      continue;
    else if(protein1.compare(protein2)>0)
     {
		 tempstr = protein1;
		 protein1 = protein2;
		 protein2 = tempstr;
	 }
	 keystr.append(protein1);
	 keystr.append(protein2);
	 if(interactionmap.find(keystr)==interactionmap.end())
		interactionmap[keystr]=1;
	 else
	 {
		 interactionmap[keystr]++;
		 continue;
	 }
    Node node1,node2;
    if(data->invIdNodeMap->find(protein1) == data->invIdNodeMap->end())
    {
      node1 = data->g->addNode();
      data->label->set(node1,protein1);
      (*(data->invIdNodeMap))[protein1] = node1;
#pragma omp critical
			{
      proteinHost[protein1] = i;// shared variable
			}
      data->nodeNum++;
    }else
    {
      node1 = (*(data->invIdNodeMap))[protein1];
    }
    if(data->invIdNodeMap->find(protein2) == data->invIdNodeMap->end())
    {
      node2 = data->g->addNode();
      data->label->set(node2,protein2);
      (*data->invIdNodeMap)[protein2] = node2;
#pragma omp critical
			{
      proteinHost[protein2] = i;
			}
      data->nodeNum++;
    }else
    {
      node2 = (*data->invIdNodeMap)[protein2];
    }

    data->g->addEdge(node1, node2);
    data->edgeNum++;
  }
  data->arcLookUpG->refresh();
  unsigned maxNode=0;
  for(NodeIt mynode(*(data->g));mynode!=lemon::INVALID;++mynode)
  {
	  for(IncEdgeIt myedge(*(data->g),mynode);myedge!=lemon::INVALID;++myedge)
	  {
		  (*data->degreeMap)[mynode]++;
	  }
	  if(maxNode < (*data->degreeMap)[mynode])
		maxNode=(*data->degreeMap)[mynode];
  }
#pragma omp critical
	{
		if(g_verbosity>=VERBOSE_NON_ESSENTIAL)
		{
			std::cerr <<filename <<" has been read successfully!"<<std::endl;
			std::cerr <<"# of proteins:"<< data->nodeNum <<"\t"<<std::endl;
			std::cerr <<"# of interactions:"<<data->edgeNum<<std::endl;
			std::cerr <<"the largest degree:"<< maxNode << std::endl;
		}
	}
//#pragma omp atomic// it also works #pragma omp critical
#pragma omp critical
	{
  allNodeNum+=data->nodeNum;// reduction
	}
  return 1;
}
#endif //NETWORKPOOL_H_
