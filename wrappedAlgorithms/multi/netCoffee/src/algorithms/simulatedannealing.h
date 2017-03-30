/* simulatedannealing.h
Author: Jialu Hu
Date: 29.06.2012*/

#ifndef SIMULATEDANNEALING_H_
#define SIMULATEDANNEALING_H_

#include <stdlib.h> //RAND_MAX 2147483647
#include <vector>
#include <random>
#include <chrono>
#include <iostream>
#include <fstream>
#include <lemon/matching.h>
#include "macro.h"
#include "time.h"
#include "verbose.h"
#include "input/recordstore.h"
#include "algorithms/solution_x.h"

template<typename RS, typename ST, typename NP, typename OP>
class SimulatedAnnealing
{
private:
    typedef RS RecordStoreType;
    typedef ST Solution_X_Type;
    typedef NP NetworkPool_Type;
    typedef OP OptionType;
    typedef typename Solution_X_Type::NodeIt NodeIt;
    typedef typename Solution_X_Type::EdgeIt EdgeIt;
    typedef typename RecordStoreType::RecordArrayValue RecordArrayValue;
    typedef typename RecordStoreType::KpGraphType KpGraph;
    typedef typename RecordStoreType::MatchingEdge MatchingEdge;
    typedef typename NetworkPool_Type::OrigLabelNodeMap OrigLabelNodeMap;
    typedef typename NetworkPool_Type::Graph Graph;
    typedef typename NetworkPool_Type::GraphData GraphData;
    typedef typename RecordStoreType::EdgeWeight EdgeWeight;
    typedef MaxWeightedMatching<Graph,EdgeWeight> MaXWeightedMatchingType;
    float _tmax;
    float _tmin;
    float _threshold;
    unsigned _Kmax;
    int _Nmax;
    float _k;
    Solution_X_Type _x;
    int _numSpecies;
    float _alpha;
    std::string _resultfolder;
public:
    SimulatedAnnealing(OptionType&);
    ~SimulatedAnnealing();
    bool initial2(RS&,NP&);
    bool initial_t(RS&,NP&);
    bool run2(RS&, NP&);
    bool run_t(KpGraph&,RS&,NP&);
    bool getMatchingEdgesAll(Graph&,RS&,MaXWeightedMatchingType&,OrigLabelNodeMap*,GraphData*,int,int,EdgeWeight*);
    bool getMatchingEdges(Graph&,MaXWeightedMatchingType&,RS&,NP&);
    bool getHighScoringMatch(Graph&,RecordStoreType&,OrigLabelNodeMap*,EdgeWeight*,int);
    void printAlignment2(std::string&,RecordStoreType&);
    void printAlignment_t(std::string&,NetworkPool_Type&);
};
template<typename RS, typename ST, typename NP, typename OP>
SimulatedAnnealing<RS,ST,NP,OP>::SimulatedAnnealing(OptionType& myoption)
:_tmax(TEMP_MAX)
,_tmin(TEMP_MIN)
,_Kmax(K_MAX)
,_Nmax(myoption.nmax)
,_k(T_CON)
,_x(myoption.alpha,myoption.numspecies)
,_numSpecies(myoption.numspecies)
,_alpha(myoption.alpha)
,_resultfolder(myoption.resultfolder)
{
	_threshold=(1-_alpha)*myoption.eta;
}

template<typename RS, typename ST, typename NP, typename OP>
SimulatedAnnealing<RS,ST,NP,OP>::~SimulatedAnnealing()
{
}

template<typename RS, typename ST, typename NP, typename OP>
bool
SimulatedAnnealing<RS,ST,NP,OP>::initial_t(RS& recordstore,NP& np)
{
  int i=0;
  recordstore.matchingEdges = new MatchingEdge();
  for(int ni=0;ni<_numSpecies;ni++)
    for(int nj=ni;nj<_numSpecies;nj++,i++)
    {
	  if(ni==nj)
	  {
		  getHighScoringMatch((*recordstore.bpgraphs[i]),recordstore,recordstore.node2labelVector[i],recordstore.edgemapVector[i],ni);
	  }
	  else
	  {
		MaXWeightedMatchingType _mwm((*recordstore.bpgraphs[i]),(*recordstore.edgemapVector[i]));
		_mwm.run();
		getMatchingEdgesAll((*recordstore.bpgraphs[i]),recordstore,_mwm,recordstore.node2labelVector[i],np.getGraph(ni),ni,nj,recordstore.edgemapVector[i]);
	  }
    }
  std::cout <<"The number of matching edges is :"<<recordstore.matchingEdges->length <<std::endl;/// The number is 114924.
  return true;
}

template<typename RS, typename ST, typename NP, typename OP>
bool
SimulatedAnnealing<RS,ST,NP,OP>::initial2(RS& recordstore,NP& np)
{
  unsigned seed =std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine generator(seed);
  std::uniform_int_distribution<unsigned> sample_distribution(0,recordstore.length-1);
  int num=0;
  _k=_T_CON;
  _Kmax=600;
  while(num<500)
  {
    unsigned select=sample_distribution(generator);
    if(g_verbosity>VERBOSE_ESSENTIAL)
      assert(select>=0 && select<recordstore.length);
    //no need to validRecord, because it has been checked when create records.
    if(!_x._isOverlapped(select,recordstore))
    {
      _x.add2(select, recordstore, np);
      num=0;
    }else
    {
      num++;
    }
  }
  if(g_verbosity>VERBOSE_ESSENTIAL)
  {
    std::cout << "Initial alignment has been built up:"<<std::endl;
    std::cout << _x.reCalculateScore2(recordstore) <<std::endl;
  }
  return true;
}

template<typename RS, typename ST, typename NP, typename OP>
bool
SimulatedAnnealing<RS,ST,NP,OP>::getMatchingEdgesAll(Graph& gr,
                                                     RS& rs,
                                                     MaXWeightedMatchingType& mwm,
                                                     OrigLabelNodeMap* node2label,
                                                     GraphData* network_1,
                                                     int ni,
                                                     int nj,
                                                     EdgeWeight* edgemap)
{
	int edgenum=0;
  for(EdgeIt e(gr);e!=lemon::INVALID;++e)
  {
    if(mwm.matching(e))
    {
			edgenum++;
      std::string protein1,protein2;
      protein1 = (*node2label)[gr.u(e)];
      protein2 = (*node2label)[gr.v(e)];
      if(network_1->invIdNodeMap->find(protein1)==network_1->invIdNodeMap->end())
      {
        std::string pro = protein1;
        protein1 = protein2;
        protein2 = pro;
      }
      if(!rs.matchingEdges->addValue(protein1,protein2,ni,nj,(*edgemap)[e]))
      {
        std::cerr << "MachingEdge pool is full now!" <<std::endl;
        return false;
      }
    }
  }
	std::cout << "Matching edges:"<<edgenum <<"\t"<<ni<<"\t"<<nj<<"\t"<<std::endl;
  return true;
}

template<typename RS, typename ST, typename NP, typename OP>
bool
SimulatedAnnealing<RS,ST,NP,OP>::getHighScoringMatch(Graph& gr,RecordStoreType& rs,OrigLabelNodeMap* node2label,EdgeWeight* edgemap,int ni)
{
	int edgenum=0;
	for(EdgeIt e(gr);e!=lemon::INVALID;++e)
	{
	  std::string protein1,protein2;
	  protein1 = (*node2label)[gr.u(e)];
      protein2 = (*node2label)[gr.v(e)];
      if(protein1.compare(protein2)==0) continue;
      if(((*edgemap)[e]-_threshold)<1e-2) continue;
			edgenum++;
      /// Do I need to test which protein is smaller? It will be tested in the next step.
      if(!rs.matchingEdges->addValue(protein1,protein2,ni,ni,(*edgemap)[e]))
      {
        std::cerr << "MachingEdge pool is full now!" <<std::endl;
        return false;
      }
	}
	std::cout <<"Number of matching edges:"<<edgenum << "\t"<<ni <<"\t"<< ni <<std::endl;
	return true;
}

template<typename RS, typename ST, typename NP, typename OP>
bool
SimulatedAnnealing<RS,ST,NP,OP>::getMatchingEdges(Graph& gr,
                                               MaXWeightedMatchingType& mwm,
                                               RecordStoreType& rs,
                                               NetworkPool_Type& networkpool)
{
  for(EdgeIt e(gr);e!=lemon::INVALID;++e)
  {
    if(mwm.matching(e))
    {
      std::string protein1,protein2;
      protein1 = rs.node2label[gr.u(e)];
      protein2 = rs.node2label[gr.v(e)];
      typename NetworkPool_Type::GraphData* network_1;
      network_1 = networkpool.getGraph(0);// The first network
      if(network_1->invIdNodeMap->find(protein1)==network_1->invIdNodeMap->end())
      {
        std::string pro = protein1;
        protein1 = protein2;
        protein2 = pro;
      }
      if(rs.edgemap[e]<_threshold)continue;
      _x.add_t(protein1,protein2,rs.edgemap[e]);
    }
  }
  return true;
}

template<typename RS, typename ST, typename NP, typename OP>
bool
SimulatedAnnealing<RS,ST,NP,OP>::run_t(KpGraph& kpgraph,RS& recordstore, NP& np)
{
  unsigned k=0;
  float t=_tmax;
  float step=(_tmax-_tmin)/_Kmax;
	std::cout <<"Initial started!"<<std::endl;
  initial_t(recordstore,np);
	std::cout <<"Initial finished!" << std::endl;
  unsigned seed =std::chrono::system_clock::now().time_since_epoch().count();
  std::string filename(_resultfolder);
  std::default_random_engine generator(seed);
  std::uniform_real_distribution<float> distribution(0.0,1.0);
  std::uniform_int_distribution<unsigned> sample_distribution(0,recordstore.matchingEdges->length-1);
  filename.append("alignmentscore.data");
  std::ofstream output(filename.c_str());
  while(k<=_Kmax)//_Kmax
  {
    t = t-step;
    float beta = 1.0/(_k*t);
    for(int n=0;n<_Nmax;++n)
    {
      float delta = 0.0;
      unsigned select=sample_distribution(generator);
      if(_x.isExist_t(select,recordstore))continue;
      int overlapValue = _x.isOverlapped_t(select,recordstore,delta);
      if(!overlapValue)/// if the matching edge is non-overlapping, ...
      {
        _x.add_t(select, recordstore);
      }
      else
      {
        if(g_verbosity>VERBOSE_ESSENTIAL)
        {
          // std::cout<< delta <<"\t"<< exp(beta*delta)<<std::endl;
        }
        if(delta>0 || distribution(generator)<exp(beta*delta))
        {
          _x.updateState_t(select, recordstore, delta, overlapValue);
        }
      }
       output << _x.getAlignmentScore() << std::endl;
    }
    std::cerr <<"Step "<<k<<"/"<<_Kmax<<"..."<< std::endl;
    k++;
  }
  std::cout <<"The final score for this alignment is:"<<_x.getAlignmentScore()<< std::endl;
  return true;
}

template<typename RS, typename ST, typename NP, typename OP>
bool
SimulatedAnnealing<RS,ST,NP,OP>::run2(RS& recordstore, NP& np)
{
  unsigned k=0;
  float t=_tmax;
  float step=(_tmax-_tmin)/_Kmax;
  std::vector<typename Solution_X_Type::Node> overlappednodes;
  std::vector<unsigned> overlappedids;
  unsigned seed =std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine generator(seed);
  std::uniform_real_distribution<float> distribution(0.0,1.0);
  std::uniform_int_distribution<unsigned> sample_distribution(0,recordstore.length-1);
  initial2(recordstore,np);
  while(k<_Kmax)
  {
    t=t-step;
    float beta=1.0/(_k*t);
    for(int n=0;n<_Nmax;++n)
    {
      unsigned select=sample_distribution(generator);///58137
      if(_x._isExist(select,recordstore))continue;
      if(!_x._isOverlapped(select,recordstore))
      {
        _x.add2(select,recordstore,np);
      }
      else
      {
        overlappednodes.clear();
        overlappedids.clear();
        float delta;
        delta=_x.slow_discrepancy2(select,recordstore,np,overlappednodes,overlappedids);
        if(g_verbosity>VERBOSE_ESSENTIAL)
        {
         // std::cerr << exp(beta*delta)<<std::endl;
        }
        if(delta>0 || distribution(generator)<exp(beta*delta))
        {
        }
        else
        {
          _x.recoverFromNew2(select,recordstore,np,overlappedids);
        }
      }
    }
    std::cerr <<"Step "<<k<<"/"<<_Kmax<<"..."<< std::endl;
    k++;
  }
  _x.reCalculateScore2(recordstore);
  std::cout <<_x.getAlignmentScore()<< "\t"<<_x.getAllNodeScore2()<<"\t"<<_x.getAllStScore()<< std::endl;
  return true;
}

template<typename RS, typename ST, typename NP, typename OP>
void
SimulatedAnnealing<RS,ST,NP,OP>::printAlignment_t(std::string& filename, NetworkPool_Type& networkpool)
{
  std::ofstream outfile1(filename.c_str());
  std::ofstream outfile2("./result/alignment_statistics.data",std::ios_base::out|std::ios_base::app);
  if(!outfile1.is_open() || !outfile2.is_open())
  {
    std::cerr << " Cannot open the outputfile! "<< std::endl;
    return;
  }
  int numNode = 0;
  for ( NodeIt it(_x.getGraph()); it!=lemon::INVALID; ++it)
  {
    std::unordered_multimap<int,std::string> *matchset;
    std::unordered_multimap<int,std::string>::iterator mit;
    matchset = _x.getNodeLabel(it);
    unsigned msize=matchset->size();
    unsigned i=0;
    if(msize>=2)
    {
      for(mit=matchset->begin();i<msize-1;mit++,i++)
      {
		  outfile1 << mit->second <<"\t" ;
	  }
	  outfile1 << mit->second << std::endl;
    }
    
    numNode++;
  }
  /// _x.getConservedEdges_t(networkpool);
  outfile2 << "----------------NETCOFFEE----------------------------" << std::endl;
  outfile2 << "The number of alignment nodes:" << numNode << std::endl;
  //for(int j=1;j<=_numSpecies;j++)
  //{
    //outfile2 <<"Alignment edges conserved over "<<j<<" species: " << _x._numConserved[j]<<std::endl;
  //}
  outfile2 << "Alpha parameter:" << _x._getAlpha() << std::endl;
  outfile2 << "nmax parameter:" << _Nmax << std::endl;
  outfile2 << "Alignment score:" << _x.getAlignmentScore() << std::endl;
  outfile1.close();
  outfile2.close();
  std::cout <<"Alignment has been written to alignment file!" <<std::endl;
  return;
}

template<typename RS, typename ST, typename NP, typename OP>
void
SimulatedAnnealing<RS,ST,NP,OP>::printAlignment2(std::string& filename, RS& rs)
{
  std::ofstream outfile4(filename.c_str());
  std::ofstream outfile5("./result/alignment_statistics.data",std::ios_base::out|std::ios_base::app);
  if(!outfile4.is_open() || !outfile5.is_open())
  {
    std::cerr << " Cannot open the outputfile! "<< std::endl;
    return;
  }
  int numNode=0;
  for(NodeIt it(_x.getGraph()); it!=lemon::INVALID; ++it)
  {
    if(!_x.validNode(it)) continue;
    unsigned select = _x.getNodeId(it);
    int i;
    for(i=0;i<_numSpecies-1;i++)
    {
      outfile4 << rs._getRecords(select,i)<<"\t";
    }
    outfile4 << rs._getRecords(select,i)<<std::endl;
    numNode++;
  }
  _x.getConservedEdges();
  outfile5 << "----------------------------------------------------" << std::endl;
  outfile5 << "The number of alignment nodes:" << numNode << std::endl;
  for(int j=0;j<_numSpecies;j++)
  {
    outfile5 <<"Alignment edges conserved over "<<j+1<<" species" << _x._numConserved[j]<<std::endl;
  }
  outfile5 << "Alpha parameter:" << _x._getAlpha() << std::endl;
  outfile5 << "nmax parameter:" << _Nmax << std::endl;
  outfile5 << "Alignment score, Node score and Edge score:" << _x.getAlignmentScore()<<"\t"<< _x.getAllNodeScore2() <<"\t"<<_x.getAllStScore() << std::endl;
  outfile4.close();
  outfile5.close();
  return;
}

//template<typename RS, typename ST, typename NP>
//void
//SimulatedAnnealing<RS,ST,NP>::printAlignment(std::string& filename, RS& rs)
//{
  //std::ofstream outfile4(filename.c_str());
  //std::ofstream outfile5("./result/alignment_statistics.data",std::ios_base::out|std::ios_base::app);
  //if(!outfile4.is_open() || !outfile5.is_open())
  //{
    //std::cerr << " Cannot open the outputfile. "<< std::endl;
    //return;
  //}
  //int numNode=0;
  //for(NodeIt it(_x.getGraph()); it!=lemon::INVALID; ++it)
  //{
    //if(!_x.validNode(it)) continue;
    //unsigned select = _x.getNodeId(it);
    //unsigned i;
    //for(i=0;i<NUM_SPECIES-1;i++)
    //{
      //outfile4 << rs.getRecords(select,i)<<"\t";
    //}
    //outfile4 << rs.getRecords(select,i)<<std::endl;
    //numNode++;
  //}
  //_x.getConservedEdges();
  //outfile5 << "----------------------------------------------------" << std::endl;
  //outfile5 << "The number of alignment nodes:" << numNode << std::endl;
  //for(int j=0;j<_numSpecies;j++)
  //{
    //outfile5 <<"Alignment edges conserved over "<<j+1<<" species" << _x._numConserved[j]<<std::endl;
  //}
  //outfile5 << "Alpha parameter:" << _x._getAlpha() << std::endl;
  //outfile5 << "Alignment score, Node score and Edge score:" << _x.reCalculateScore(rs)<<"\t"<< _x.getAllNodeScore() <<"\t"<<_x.getAllStScore() << std::endl;
  //outfile4.close();
  //outfile5.close();
  //return;
//}

//template<typename RS, typename ST, typename NP>
//void
//SimulatedAnnealing<RS,ST,NP>::_printAlignment(std::string& filename, RS& rs)
//{
  //std::ofstream outfile4("./result/alignment.network.data");
  //std::ofstream outfile5("./result/alignment_statistics.data",std::ios_base::out|std::ios_base::app);
  //if(!outfile4.is_open() || !outfile5.is_open())
  //{
    //std::cerr << " Cannot open the outputfile. "<< std::endl;
    //return;
  //}
  //int numNode=0;
  //for(NodeIt it(_x.getGraph()); it!=lemon::INVALID; ++it)
  //{
    //if(!_x.validNode(it)) continue;
    //unsigned select = _x.getNodeId(it);
    //int i;
    //for(i=0;i<_numSpecies-1;i++)
    //{
      //outfile4 << rs._getRecords(select,i)<<"\t";
    //}
    //outfile4 << rs._getRecords(select,i)<<std::endl;
    //numNode++;
  //}
  
  //outfile5 << "----------------------------------------------------" << std::endl;
  //outfile5 << "The number of alignment nodes:" << numNode << std::endl;
  //outfile5 << "Alpha parameter:" << _x._getAlpha() << std::endl;
  //outfile5 << "Alignment score, Node score and Edge score:" << _x.getAlignmentScore()<<"\t"<< _x._getAllNodeScore() <<"\t" << _x._getAllStScore() << std::endl;
  //outfile4.close();
  //outfile5.close();
  //return;
//}
#endif //SIMULATEDANNEALING_H_
