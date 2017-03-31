/* solution_x.h
Author: Jialu Hu
Date: 29.06.2012*/

#ifndef SOLUTION_X_H_
#define SOLUTION_X_H_

#include <lemon/core.h>
#include <lemon/bits/graph_extender.h>
#include <lemon/concepts/graph.h>
#include <unordered_map>
#include <array>
#include <vector>
#include <algorithm>
#include "macro.h"
#include "verbose.h"
#include "input/recordstore.h"
#include "input/networkpool.h"

template<typename GR,typename Con,typename BP, typename Option>
class Solution_X
{
private:    
  typedef GR Graph;
  typedef NetworkPool<GR,BP> NetworkPool_Type;
  typedef RecordStore<Con,Option> RecordStore_Type;
  typedef typename NetworkPool<GR, BP>::GraphData GraphData;
  typedef typename RecordStore_Type::RecordArrayValue RecordArrayValue;
  typedef typename RecordStore_Type::TripletArray TripletArray;
  typedef typename RecordStore_Type::MatchingEdge MatchingEdge;
  typedef typename RecordStore_Type::Proteins Proteins;
  typedef typename RecordStore_Type::RecordArray RecordArray;
  typedef typename RecordStore_Type::KpGraphType KpGraph;
  typedef typename KpGraph::BpGraph BpGraph;
  typedef typename KpGraph::EdgeMap EdgeMap;
public:
  TEMPLATE_GRAPH_TYPEDEFS(Graph);

  /// Labels of the nodes.
  typedef typename Graph::template NodeMap<unsigned int> Node2LabelMap;
  /// Mapping from nodes to vector of string.
  typedef typename Graph::template NodeMap<std::unordered_multimap<int,std::string>* > Node2StringMap;/// This should be an unordered_multipmap
  /// Mapping from labels to original nodes.
  typedef std::unordered_map<std::string, typename Graph::Node> Label2NodeMap;
  /// Weights on original edges.
  typedef typename Graph::template EdgeMap<float> WeightEdgeMap;
  Graph *_g;
  Node2LabelMap *_node2label;
  Node2StringMap *_node2string;
  Label2NodeMap *_label2node1;
  Label2NodeMap *_label2node2;
  Label2NodeMap *_label2node3;
  Label2NodeMap *_label2node;
  WeightEdgeMap *_weightmap;
  ArcLookUp<Graph> *_arcLookUp;
public:  
  float _allNodeScore;// sequence similarity score
  float _allEdgeScore;// topology score mnetaligner
  float _allStScore;// topology score tcoffee
  float _maxNodeScore;
  unsigned _maxStScore;
  float _alignmentScore;// alignment score
  float _alpha;
  unsigned _numNode;
  unsigned _numEdge;
  unsigned* _numConserved;
  RecordArray recordarray;
  int _numSpecies;
  typedef struct _DeltaData{
    float deltaNodes;
    float deltaEdges;
    unsigned deltaNumEdge;
    _DeltaData():deltaNodes(0.0),deltaEdges(0.0),deltaNumEdge(0)
    {
    }
    void tozero(){deltaNodes=0.0;deltaEdges=0.0;deltaNumEdge=0;}
  }DeltaData;
  DeltaData _delta;

  Solution_X(double,int);
  ~Solution_X();
  int __isOverlapped(unsigned int,RecordArrayValue*,unsigned);
  bool _isExist(unsigned,RecordStore_Type&);
  bool isExist_t(unsigned,RecordStore_Type&);
  bool isOverlapped(unsigned,RecordStore_Type&);
  bool _isOverlapped(unsigned,RecordStore_Type&);
  int isOverlapped_t(unsigned,RecordStore_Type&,float&);
  bool isCombine(std::unordered_multimap<int, std::string>&,RecordStore_Type&,std::string,int);
  float getDeltaScore(std::unordered_multimap<int, std::string>&,std::unordered_multimap<int, std::string>&,RecordStore_Type&);
  bool add(unsigned, RecordStore_Type&, NetworkPool<GR,BP>&);
  bool add2(unsigned, RecordStore_Type&, NetworkPool<GR,BP>&);
  bool _add(unsigned, RecordStore_Type&, NetworkPool<GR,BP>&);
  bool add_t(unsigned,RecordStore_Type& rs);
  bool addRecord_t(unsigned,RecordStore_Type&);
  Label2NodeMap* getLabel2Node(unsigned i);
  float calculateScore();
  bool createTriplet(RecordStore_Type&,KpGraph&,NetworkPool_Type&);
  bool validRecord(unsigned, RecordStore_Type&, NetworkPool<GR,BP>&);
  float discrepancy(unsigned, RecordStore_Type&, NetworkPool<GR, BP>&,std::vector<Node>&);
  float _discrepancy(unsigned, RecordStore_Type&, NetworkPool<GR, BP>&,std::vector<Node>&);
  float slow_discrepancy(unsigned, RecordStore_Type&, NetworkPool<GR, BP>&,
    std::vector<Node>&,std::vector<unsigned>&);
  float slow_discrepancy2(unsigned, RecordStore_Type&, NetworkPool<GR, BP>&,
    std::vector<Node>&,std::vector<unsigned>&);
  bool updateState(unsigned, RecordStore_Type&, NetworkPool<GR, BP>&,std::vector<Node>&);
  bool updateState2(unsigned, RecordStore_Type&, NetworkPool<GR, BP>&,std::vector<Node>&);
  bool updateState_t(unsigned, RecordStore_Type&,float,int);
  bool _updateState(unsigned, RecordStore_Type&, NetworkPool<GR, BP>&,std::vector<Node>&);
  float reCalculateScore(RecordStore_Type&);
  float reCalculateScore2(RecordStore_Type&);
  bool _connnectAlignmentGraph(RecordStore_Type&, NetworkPool<GR,BP>&);
  bool __isOccupied(unsigned ,unsigned);
  float _reCalculateScore();
  float _getAllNodeScore();
  float _getAllStScore();
  float getAllNodeScore();
  float getAllNodeScore2();
  float getAllStScore();
  float getAlignmentScore();
  float _getAlpha();
  void removeNode(unsigned,RecordStore_Type&);
  void removeNode2(unsigned,RecordStore_Type&);
  Graph& getGraph();
  bool validNode(Node&);
  unsigned getNodeId(Node&);
  std::unordered_multimap<int,std::string>* getNodeLabel(Node&);
  void recoverFromNew(unsigned,
    RecordStore_Type&,
    NetworkPool<GR,BP>&,
    std::vector<unsigned>&);
  void recoverFromNew2(unsigned,
    RecordStore_Type&,
    NetworkPool<GR,BP>&,
    std::vector<unsigned>&);
  void getConservedEdges();
  void getConservedEdges_t(NetworkPool_Type&);
  void printDecomposedScore();
};

template<typename GR,typename Con,typename BP, typename Option>
Solution_X<GR,Con,BP,Option>::Solution_X(double alpha,int numspecies)
  :_allNodeScore(0.0)
  ,_allEdgeScore(0.0)
  ,_allStScore(0.0)
  ,_maxNodeScore(0.0)
  ,_maxStScore(0)
  ,_alignmentScore(0.0)
  ,_alpha(alpha)
  ,_numNode(0)
  ,_numEdge(0)
  ,_numSpecies(numspecies)
  ,_delta()
{
  _g = new Graph();
  _node2label=new Node2LabelMap(*_g);
  _node2string=new Node2StringMap(*_g);
  _label2node1=new Label2NodeMap();
  _label2node2=new Label2NodeMap();
  _label2node3=new Label2NodeMap();
  _label2node =new Label2NodeMap();
  _numConserved = new unsigned [_numSpecies+1];
  _weightmap=new WeightEdgeMap(*_g);
  _arcLookUp=new ArcLookUp<Graph>(*_g);
  _g->reserveNode(NUM_NODE_REVERSE);
  _g->reserveEdge(NUM_EDGE_REVERSE);
  //recordarray = new Proteins[NUM_SPECIES];
}
template<typename GR,typename Con,typename BP, typename Option>
Solution_X<GR,Con,BP,Option>::~Solution_X()
{
  delete _g;
  delete _node2label;
  delete _label2node1;
  delete _label2node2;
  delete _label2node3;
  delete _weightmap;
  delete _arcLookUp;
  delete [] _numConserved;
  //for(typename Node2StringMap::iterator it=_node2string->begin();it!=_node2string->end();++it)
  //{
  //delete it->second;
  //}
  delete _node2string;
}

template<typename GR,typename Con,typename BP, typename Option>
std::unordered_map<std::string, typename GR::Node>*
  Solution_X<GR,Con,BP,Option>::getLabel2Node(unsigned i)
{
  Label2NodeMap* nodemap=NULL;
  switch(i)
  {
  case 0: nodemap=_label2node1;break;
  case 1: nodemap=_label2node2;break;
  case 2: nodemap=_label2node3;break;
  default: ;
  }
  return nodemap;
}

template<typename GR,typename Con,typename BP, typename Option>
bool
  Solution_X<GR,Con,BP,Option>::__isOccupied(unsigned k,unsigned id)
{
  //std::string protein=
  return true;
}

template<typename GR,typename Con,typename BP, typename Option>
int
  Solution_X<GR,Con,BP,Option>::__isOverlapped(unsigned int select,RecordArrayValue* rs,unsigned k)
{
  if(k==1)
  {
  }
  else
  {
    if((*_label2node).find(rs->recordArray[0][select])!=(*_label2node).end() &&
      (*_label2node).find(rs->recordArray[1][select])!=(*_label2node).end())
    {
      Node node1=(*_label2node)[rs->recordArray[0][select]];
      Node node2=(*_label2node)[rs->recordArray[1][select]];
      if(node1!=node2)return -1;
      if((*_label2node).find(rs->recordArray[2][select])!=(*_label2node).end())
      {
        Node node3=(*_label2node)[rs->recordArray[2][select]];
        if(node1==node3)return 3;
        else return 1;
      }
      ///else return 2;
      else
      {
        unsigned id = getNodeId(node1);
        if(__isOccupied(k+1,id))return 1;
        else return 0;
      }
    }
    else if((*_label2node).find(rs->recordArray[0][select])==(*_label2node).end() &&
      (*_label2node).find(rs->recordArray[1][select])==(*_label2node).end())
    {
      if((*_label2node).find(rs->recordArray[2][select])!=(*_label2node).end())
        return 1;/// record is overlapped
      else
        return 0;/// record is not overlapped
    }
    return -1;/// record is invalid
  }
  return -1;
}

template<typename GR,typename Con,typename BP, typename Option>
bool
  Solution_X<GR,Con,BP,Option>::_isOverlapped(unsigned int select,RecordStore_Type& rs)
  /// This function can be used on both mnetaligner and tCoffee.
{
  for(int i=0;i<_numSpecies;i++)
  {
    if((*_label2node).find(rs.recordarray[i][select])!=(*_label2node).end())
    {
      Node node=(*_label2node)[rs.recordarray[i][select]];
      if(validNode(node))/// In case, node was deleted in the process of updateState.
        return true;
    }
  }
  return false;
}

template<typename GR,typename Con,typename BP, typename Option>
float
  Solution_X<GR,Con,BP,Option>::getDeltaScore(std::unordered_multimap<int, std::string>& s1,//old matchingset
  std::unordered_multimap<int, std::string>& s2,//new matchingset
  RecordStore_Type& rs)
{
  typedef std::unordered_multimap<int, std::string>::iterator Iterator;
  float score1,score2;
  score1=0.0;score2=0.0;
  for(Iterator it1=s1.begin();it1!=s1.end();it1++)
  {
    Iterator mirror=it1;
    mirror++;
    for(Iterator it2=mirror;it2!=s1.end();it2++)
    {
      std::string keystring,protein1,protein2;
      protein1=it1->second;
      protein2=it2->second;
      short species1,species2;
      species1=rs.matchingEdges->getSpecies(protein1);
      species2=rs.matchingEdges->getSpecies(protein2);
      if(species1==species2 && protein1.compare(protein2)>0 )
      {
        std::string temp(protein1);
        protein1=protein2;
        protein2=temp;
      }
      keystring.append(protein1);keystring.append(protein2);
      if(rs.matchingEdges->isMatchingEdge(keystring))
        score1+=rs.matchingEdges->getMatchingScore(keystring);
    }
  }

  for(Iterator it1=s2.begin();it1!=s2.end();it1++)
  {
    Iterator mirror=it1;
    mirror++;
    for(Iterator it2=mirror;it2!=s2.end();it2++)
    {
      std::string keystring,protein1,protein2;
      protein1=it1->second;
      protein2=it2->second;
      short species1,species2;
      species1=rs.matchingEdges->getSpecies(protein1);
      species2=rs.matchingEdges->getSpecies(protein2);
      if(species1==species2 && protein1.compare(protein2)>0 )
      {
        std::string temp(protein1);
        protein1=protein2;
        protein2=temp;
      }
      keystring.append(protein1);keystring.append(protein2);
      if(rs.matchingEdges->isMatchingEdge(keystring))
        score2+=rs.matchingEdges->getMatchingScore(keystring);
    }
  }
  return score1 - score2;
}
template<typename GR,typename Con,typename BP, typename Option>
int
  Solution_X<GR,Con,BP,Option>::isOverlapped_t(unsigned int select,RecordStore_Type& rs, float& myDeltaScore)
{
  typedef std::unordered_multimap<int, std::string>::iterator Iterator;
  int overlapValue=0;
  std::string protein1,protein2;
  std::string keystring;
  std::unordered_multimap<int, std::string>  *ms1,*ms2;
  protein1 = rs.matchingEdges->getValue(0,select);/// stop here.
  protein2 = rs.matchingEdges->getValue(1,select);
  keystring.append(protein1);
  keystring.append(protein2);
  short species1 = rs.matchingEdges->getSpecies(protein1);
  short species2 = rs.matchingEdges->getSpecies(protein2);
  float deltaScore[10]={0};
  if(_label2node->find(protein1)!=_label2node->end() &&
    _label2node->find(protein2)!=_label2node->end())
  {
    Node node1 = (*_label2node)[protein1];
    Node node2 = (*_label2node)[protein2];
    ms1  = (*_node2string)[node1];
    ms2  = (*_node2string)[node2];
    std::unordered_multimap<int, std::string> newmp1(*ms1);
    std::unordered_multimap<int, std::string> newmp2(*ms2);
    int index_i,index_j;

    if(isCombine(*ms1,rs,protein2,species2))/// shuold I combine protein2 to matchset containing protein 1
      /// isCombine(std::unordered_multimap<int, std::string>& matchset,RecordStore_Type& rs,std::string protein,int species)
    {
      newmp1.insert (std::make_pair(species2,protein2)); /// combine protein2 to matchset 1
      auto range = newmp2.equal_range(species2);
      for(Iterator it = range.first;it != range.second; it++)
      {
        if(it->second.compare(protein2)==0){
          newmp2.erase(it);/// erase protein2 from matchset 2
          break;}
      }
      deltaScore[1]=getDeltaScore(newmp1,*ms1, rs)+getDeltaScore(newmp2,*ms2, rs);
      index_i=1;
    }//  get deltaScore 1
    else
    {
      auto range = newmp1.equal_range(species2);
      Iterator it = range.first;
      assert(it!=newmp1.end());
        it->second = protein2; /// substitute the first protein with protein2 in matchset 1
        range = newmp2.equal_range(species2);
        for(Iterator it = range.first;it != range.second; it++)
        {
          if(it->second.compare(protein2)==0){
            newmp2.erase(it);/// erase protein2 from matchset 2
            break;}
        }
        deltaScore[2] =getDeltaScore(newmp1,*ms1, rs)+getDeltaScore(newmp2,*ms2, rs);
        index_i=2;
    }
    newmp1=*ms1;newmp2=*ms2;
    //  get deltaScore 2
    if(isCombine(*ms2,rs,protein1,species1)) /// shuold I combine protein1 to matchset containing protein 2
    {
      newmp2.insert (std::make_pair(species1,protein1));/// combine protein1 to matchset 2
      auto range = newmp1.equal_range(species1);
      for(Iterator it = range.first;it != range.second; it++)
      {
        if(it->second.compare(protein1)==0){
          newmp1.erase(it);/// erase protein1 from matchset 1
          break;}
      }
      deltaScore[3]=getDeltaScore(newmp2,*ms2, rs)+getDeltaScore(newmp1,*ms1, rs);
      index_j=3;
    }//  get deltaScore 3
    else
    {
      auto range = newmp2.equal_range(species1);
      Iterator it = range.first;
      assert(it!=newmp2.end());
      it->second = protein1; /// substitute the first protein with protein1 in matchset 2
      range = newmp1.equal_range(species1);
      for(it = range.first;it != range.second; it++)
      {
        if(it->second.compare(protein1)==0){
          newmp1.erase(it);/// erase protein1 from matchset 1
          break;}
      }
      deltaScore[4] =getDeltaScore(newmp1,*ms1, rs)+getDeltaScore(newmp2,*ms2, rs);
      index_j=4;
      //  get deltaScore 4
    }
    /// compare the four deltaScore generated above choose the biggest one and set overlapValue
    float maxDelta = deltaScore[index_i]; overlapValue=index_i;
    if(maxDelta < deltaScore[index_j])
    {
      maxDelta = deltaScore[index_j];
      overlapValue = index_j;
    }
  }
  else if(_label2node->find(protein1)!=_label2node->end())
  {
    Node node1 = (*_label2node)[protein1];
    ms1  = (*_node2string)[node1];
    std::unordered_multimap<int, std::string> newmp1(*ms1);
    if(isCombine(*ms1,rs,protein2,species2))/// should I combine protein2 to matchset containing protein1;
    {
      newmp1.insert (std::make_pair(species2,protein2));/// combine protein2 to matchset 1
      deltaScore[5]=getDeltaScore(newmp1,*ms1, rs);
      overlapValue=5;
    }/// get deltaScore 5
    else
    {
      auto range = newmp1.equal_range(species2);
      Iterator it = range.first;
      assert(it!=newmp1.end());
      it->second = protein2; /// substitute the first protein with protein2 in matchset 1
      deltaScore[6] =getDeltaScore(newmp1,*ms1, rs);
      overlapValue=6;
    }
    /// get deltaScore 6
  }
  else if(_label2node->find(protein2)!=_label2node->end())
  {
    Node node2 = (*_label2node)[protein2];
    ms2  = (*_node2string)[node2];
    std::unordered_multimap<int, std::string> newmp2(*ms2);
    if(isCombine(*ms2,rs,protein1,species1)) /// shuold I combine protein1 to matchset containing protein 2
    {
      newmp2.insert (std::make_pair(species1,protein1));/// combine protein1 to matchset 2
      deltaScore[7]=getDeltaScore(newmp2,*ms2, rs);
      overlapValue=7;
    }/// get deltaScore 7
    else{
	    auto range = newmp2.equal_range(species1);
	    Iterator it = range.first;
	    assert(it!=newmp2.end());
	    it->second = protein1; /// substitute the first protein with protein2 in matchset 1
	    deltaScore[8] =getDeltaScore(newmp2,*ms2, rs);
	    overlapValue=8;
	}
    /// get deltaScore 8
  }
  myDeltaScore = deltaScore[overlapValue];
  return overlapValue;
}

template<typename GR,typename Con,typename BP, typename Option>
bool
  Solution_X<GR,Con,BP,Option>::isCombine(std::unordered_multimap<int, std::string>& matchset,RecordStore_Type& rs,std::string protein,int species)
{
  typedef std::unordered_multimap<int, std::string>::iterator Iterator;
  auto range = matchset.equal_range(species);
  if(range.first==matchset.end())return true;/// if no protein in matchset has the same species num. with protein, then combine it.
  for(Iterator it = range.first;it != range.second; it++)
  {
    std::string keystr,protein1,protein2;
    protein1=protein;protein2= it->second ;
    if(protein1.compare(protein2)>0)
    {
      std::string temp;
      temp=protein1;
      protein1=protein2;
      protein2=temp;				
    }
    keystr.append(protein1);keystr.append(protein2);
    if(!rs.matchingEdges->isMatchingEdge(keystr)) return false;
  }
  return true;/// if it is in that case, substitute it with protein.
}

template<typename GR,typename Con,typename BP, typename Option>
bool
  Solution_X<GR,Con,BP,Option>::isOverlapped(unsigned int select,RecordStore_Type& rs)
{
  if((*_label2node1).find(rs.proteins1->at(select))!=(*_label2node1).end() ||
    (*_label2node2).find(rs.proteins2->at(select))!=(*_label2node2).end() ||
    (*_label2node3).find(rs.proteins3->at(select))!=(*_label2node3).end()
    )
    return true;
  else
    return false;
}

template<typename GR,typename Con,typename BP, typename Option>
bool
  Solution_X<GR,Con,BP,Option>::_connnectAlignmentGraph(RecordStore_Type& rs, NetworkPool<GR,BP>& np)
{
  for(NodeIt it(*_g);it!=lemon::INVALID;++it)
  {
    for(unsigned i=0;i<_numSpecies;i++)
    {
      unsigned select=(*_node2label)[it];
      //std::string protein=rs
    }
  }
  return true;
}

template<typename GR,typename Con,typename BP, typename Option>
bool
  Solution_X<GR,Con,BP,Option>::_add(unsigned int select, RecordStore_Type& rs, NetworkPool<GR,BP>& np)
{
  Node node=_g->addNode();
  float seScore,stScore;
  for(int i=0;i<_numSpecies;i++)
  {
    std::string protein=rs.recordarray[i][select];
    (*_label2node)[protein]=node;
  }
  (*_node2label)[node]=select;
  seScore = rs._getRecordSeScore(select);
  stScore = rs._getRecordStScore(select);
  _allNodeScore += seScore;// sequence similarity score
  _allStScore += stScore;// topology similarity score
  _numNode++;
  _reCalculateScore();
  return true;
}

template<typename GR,typename Con,typename BP, typename Option>
bool
  Solution_X<GR,Con,BP,Option>::add_t(unsigned select, RecordStore_Type& rs)
{
  typedef std::unordered_multimap<int, std::string> MultiMap;
  Node node;
  node = _g->addNode();
  MultiMap* pt=new MultiMap(); 
  std::string protein1,protein2;
  std::string keystring;
  protein1 = rs.matchingEdges->getValue(0,select);
  protein2 = rs.matchingEdges->getValue(1,select);
  short species1 = rs.matchingEdges->getSpecies(protein1);
  short species2 = rs.matchingEdges->getSpecies(protein2);
  if(species1==species2 && protein1.compare(protein2)>0)
  {
    std::string temp(protein1);
    protein1=protein2;
    protein2=temp;
  }
  keystring.append(protein1);
  keystring.append(protein2);
  pt->insert(make_pair(species1,protein1));
  pt->insert(make_pair(species2,protein2));
  (*_node2string).set(node,pt);
  (*_label2node)[protein1] = node;
  (*_label2node)[protein2] = node;
  _alignmentScore+=rs.matchingEdges->getMatchingScore(keystring);
  return true;
}

template<typename GR,typename Con,typename BP, typename Option>
bool
  Solution_X<GR,Con,BP,Option>::addRecord_t(unsigned select, RecordStore_Type& rs)
{
  //std::string protein1,protein2;
  //protein1 = rs.matchingEdges->getValue(0,select);
  //protein2 = rs.matchingEdges->getValue(1,select);
  //short species = rs.matchingEdges->getSpecies(protein2);
  //Node node = (*_label2node)[protein1];
  //(*_node2string)[node][species] = protein3;
  //(*_label2node)[protein3] = node;
  //protein1.append(protein3);
  //_alignmentScore+=rs.matchingEdges->getTripletScore(protein1);
  return true;
}

template<typename GR,typename Con,typename BP, typename Option>
bool
  Solution_X<GR,Con,BP,Option>::add2(unsigned int select, RecordStore_Type& rs, NetworkPool<GR,BP>& np)
  /// This function is used for mnetaligner on multiple networks.
{
  Node node=_g->addNode();
  for(int i=0;i<_numSpecies;i++)
  {
    std::string protein=rs.recordarray[i][select];
    (*_label2node)[protein]=node;
  }
  (*_node2label)[node]=select;
  /// Add conserved edges to alignment graph.
  for(int i=0;i<_numSpecies;i++)
  {
    GraphData *graph;
    graph = np.getGraph(i);
    for(IncEdgeIt e(*(graph->g),(*(graph->invIdNodeMap))[rs.recordarray[i][select]]);
      e!=lemon::INVALID; ++e)// Protein i might not exist in graph
    {
      Node neighbor=(graph->g)->runningNode(e);
      std::string pro=(*graph->label)[neighbor];
      if(rs.recordarray[i][select]==pro)continue;// In case, self-edge is found.
      if(_label2node->find(pro)!=_label2node->end())
      {
        Node existNode=(*_label2node)[pro];
        IncEdgeIt en(*_g,node);
        for(; en!=lemon::INVALID; ++en)
        {
          if(_g->runningNode(en)==existNode)break;
        }
        if(en!=lemon::INVALID)
        {
          (*_weightmap)[en]+=1.0/_numSpecies;
        }
        else
        {
          Edge newedge=_g->addEdge(node,existNode);
          (*_weightmap)[newedge]=1.0/_numSpecies;
        }
      }
    }
  }
  return true;
}

template<typename GR,typename Con,typename BP, typename Option>
bool
  Solution_X<GR,Con,BP,Option>::add(unsigned int select, RecordStore_Type& rs, NetworkPool<GR,BP>& np)
{
  Node node=_g->addNode();
  std::string proteins[NUM_SPECIES];
  proteins[0]=rs.proteins1->at(select);
  proteins[1]=rs.proteins2->at(select);
  proteins[2]=rs.proteins3->at(select);
  (*_node2label)[node]=select;
  (*_label2node1)[proteins[0]]=node;
  (*_label2node2)[proteins[1]]=node;
  (*_label2node3)[proteins[2]]=node;
  float score=rs.getRecordScore(select);
  if(_maxNodeScore < score)
  {
    _maxNodeScore=score;
  }
  _numNode++;
  for(unsigned i=0;i<NUM_SPECIES;i++)
  {
    GraphData *graph;
    graph = np.getGraph(i);
    Label2NodeMap* label2Node=getLabel2Node(i);    
    for(IncEdgeIt e(*(graph->g),(*(graph->invIdNodeMap))[proteins[i]]);
      e!=lemon::INVALID; ++e)//protein i might not exist in graph
    {
      Node neighbor=(graph->g)->runningNode(e);
      std::string pro=(*graph->label)[neighbor];
      if(proteins[i]==pro)continue;
      if(label2Node->find(pro)!=label2Node->end())
      {
        Node existNode=(*label2Node)[pro];
        IncEdgeIt en(*_g,node);
        for(; en!=lemon::INVALID; ++en)
        {
          if(_g->runningNode(en)==existNode)break;
        }
        if(en!=lemon::INVALID)
        {
          (*_weightmap)[en]+=1.0/NUM_SPECIES;
        }
        else
        {
          Edge newedge=_g->addEdge(node,existNode);
          (*_weightmap)[newedge]=1.0/NUM_SPECIES;
        }
      }
    }
  }
  return true;
}

template<typename GR,typename Con,typename BP, typename Option>
bool
  Solution_X<GR,Con,BP,Option>::validRecord(unsigned int select, RecordStore_Type& rs, NetworkPool<GR,BP>& np)
{
  std::string proteins[NUM_SPECIES];
  proteins[0]=rs.proteins1->at(select);
  proteins[1]=rs.proteins2->at(select);
  proteins[2]=rs.proteins3->at(select);  
  for(unsigned i=0;i<NUM_SPECIES;i++)
  {
    GraphData *graph;
    graph = np.getGraph(i);
    if(graph->invIdNodeMap->find(proteins[i])==graph->invIdNodeMap->end())
    {
      if(g_verbosity>VERBOSE_ESSENTIAL)
      {
        //std::cout << "Records "<<proteins[i]<<" donot exist in networks" <<std::endl;
      }
      return false;
    }
  }
  return true;
}
template<typename GR,typename Con,typename BP, typename Option>
float
  Solution_X<GR,Con,BP,Option>::calculateScore()
{
  //std::cout <<"MaxNode "<<_maxNodeScore <<"AllNode " <<_allNodeScore << "AllEdge "<<_allEdgeScore<<std::endl;
  //std::cout <<"NumNode "<<_numNode<<"NumEdge "<<_numEdge<<std::endl;
  _alignmentScore=_alpha*(_allNodeScore/_maxNodeScore)+(1-_alpha)*_allEdgeScore;
  //std::cout<<"error"<<std::endl;
  //std::cout <<"alignmentScore "<<_alignmentScore<<std::endl;
  return _alignmentScore;
}


template<typename GR,typename Con,typename BP, typename Option>
bool
  Solution_X<GR,Con,BP,Option>::createTriplet(RecordStore_Type& rs, KpGraph& kpgraph,NetworkPool_Type& networkpool)
{
  GraphData* network_k;
  rs.tripletRecord = new TripletArray();// Allocate memory for tripletRecord.
  std::ofstream output(rs.recordfile.c_str());
  for(int i=1;i<_numSpecies-1;i++)
  {
    std::string protein1,protein2,protein3,kst1,kst2,kst3;
    float evalue2,evalue3;
    unsigned st_score2,st_score3;
    network_k = networkpool.getGraph(i+1);// The third network
    BpGraph* bp1k=kpgraph.graphs[i];
    BpGraph* bp2k=kpgraph.graphs[i+_numSpecies-2];
    for(NodeIt it(*_g); it != lemon::INVALID; ++it)
    {
      protein1 = (*_node2string)[it][0];
      protein2 = (*_node2string)[it][1];
      auto range=bp1k->redBlue.equal_range(protein1);
      typename EdgeMap::iterator pt;
      for(pt=range.first;pt!=range.second;++pt)
      {
        protein3=pt->second;
        if(!kpgraph.isEdge(protein3,network_k))continue;
        kst2.clear();kst3.clear();
        kst2.append(protein1);kst2.append(protein3);
        kst3.append(protein2);kst3.append(protein3);
        if(bp2k->stWeight.find(kst3)!=bp2k->stWeight.end())
        {
          st_score2 = bp1k->stWeight[kst2];
          st_score3 = bp2k->stWeight[kst3];
          evalue2 =rs.getScore(bp1k->seWeight[kst2]);
          evalue3 =rs.getScore(bp2k->seWeight[kst3]);
          float weight = rs.combineScore(evalue2,st_score2)+rs.combineScore(evalue3,st_score3);
          rs.tripletRecord->addValue(protein1,protein2,protein3,i+1,weight);
          output << protein1 <<"\t" << protein2 <<"\t" << protein3 <<"\t"<<st_score2+st_score3<<"\t"<<evalue2+evalue3<<"\t"<<weight <<"\n";
        }
      }
    }
  }
  return true;
}

template<typename GR,typename Con,typename BP, typename Option>
float
  Solution_X<GR,Con,BP,Option>::_reCalculateScore()
{
  _alignmentScore=_alpha*(_allNodeScore)+(1-_alpha)*(_allStScore);
  return _alignmentScore;
}

template<typename GR,typename Con,typename BP, typename Option>
float
  Solution_X<GR,Con,BP,Option>::_getAllNodeScore()
{
  return _alpha*_allNodeScore;
}

template<typename GR,typename Con,typename BP, typename Option>
float
  Solution_X<GR,Con,BP,Option>::getAllNodeScore()
{
  return _alpha*(_allNodeScore/_maxNodeScore);
}

template<typename GR,typename Con,typename BP, typename Option>
float
  Solution_X<GR,Con,BP,Option>::getAllNodeScore2()
{
  return _alpha*_allNodeScore;
}

template<typename GR,typename Con,typename BP, typename Option>
float
  Solution_X<GR,Con,BP,Option>::_getAllStScore()
{
  return (1-_alpha)*_allStScore;
}

template<typename GR,typename Con,typename BP, typename Option>
float
  Solution_X<GR,Con,BP,Option>::getAllStScore()
{
  return (1-_alpha)*_allEdgeScore;
}

template<typename GR,typename Con,typename BP, typename Option>
float
  Solution_X<GR,Con,BP,Option>::getAlignmentScore()
{
  return _alignmentScore;
}

template<typename GR,typename Con,typename BP, typename Option>
float
  Solution_X<GR,Con,BP,Option>::_getAlpha()
{
  return _alpha;
}

template<typename GR,typename Con,typename BP, typename Option>
float
  Solution_X<GR,Con,BP,Option>::reCalculateScore2(RecordStore_Type& rs)
{
  _allNodeScore=0.0;
  _allEdgeScore=0.0;
  _numNode=0;
  _numEdge=0;
  for(NodeIt it(*_g);it!=lemon::INVALID;++it,++_numNode)
  {
    if(!_g->valid(it))continue;
    _allNodeScore+=rs._getRecordSeScore((*_node2label)[it]);
  }
  for(EdgeIt e(*_g);e!=lemon::INVALID;++e,++_numEdge)
  {
    if(!_g->valid(e))continue;
    _allEdgeScore+=(*_weightmap)[e];
  }
  _alignmentScore=_alpha*(_allNodeScore)+(1-_alpha)*_allEdgeScore;
  return _alignmentScore;
}

template<typename GR,typename Con,typename BP, typename Option>
float
  Solution_X<GR,Con,BP,Option>::reCalculateScore(RecordStore_Type& rs)
{
  _allNodeScore=0.0;
  _allEdgeScore=0.0;
  for(NodeIt it(*_g);it!=lemon::INVALID;++it)
  {
    if(!_g->valid(it))continue;
    _allNodeScore+=rs.getRecordScore((*_node2label)[it]);
  }
  for(EdgeIt e(*_g);e!=lemon::INVALID;++e)
  {
    if(!_g->valid(e))continue;
    _allEdgeScore+=(*_weightmap)[e];
  }
  _alignmentScore=_alpha*(_allNodeScore/_maxNodeScore)+(1-_alpha)*_allEdgeScore;
  ///std::cout << "nodescore "<<_alpha*(nodeScore/_maxNodeScore) <<"edgescore "<<
  ///(1-_alpha)*edgeScore<<std::endl;
  return _alignmentScore;
}

// To calculate the discrepancy after using tcoffee technique.
template<typename GR,typename Con,typename BP, typename Option>
float
  Solution_X<GR,Con,BP,Option>::_discrepancy(unsigned select,
  RecordStore_Type& rs,
  NetworkPool<GR,BP>& np,
  std::vector<Node>& overlappednodes)
{
  float seScore=0.0;
  float stScore=0.0;
  seScore+=rs._getRecordSeScore(select);
  stScore+=rs._getRecordStScore(select);
  for(unsigned i=0; i<NUM_SPECIES; i++)
  {
    std::string protein=rs._getRecords(select, i);
    if(_label2node->find(protein)!=_label2node->end())
    {
      Node node=(*_label2node)[protein];
      assert(_g->valid(node));
      if(std::find(overlappednodes.begin(),overlappednodes.end(),node)==overlappednodes.end())
      {
        unsigned id=(*_node2label)[node];
        seScore -=rs._getRecordSeScore(id);
        stScore -=rs._getRecordStScore(id);
        overlappednodes.push_back(node);
      }
    }
  }
  return _alpha*seScore+(1-_alpha)*static_cast<float>(stScore);
}

template<typename GR,typename Con,typename BP, typename Option>
bool
  Solution_X<GR,Con,BP,Option>::_isExist(unsigned select,RecordStore_Type& rs)
{
  std::string protein=rs._getRecords(select,0);
  if(_label2node->find(protein)!=_label2node->end())
  {
    Node node=(*_label2node)[protein];
    if(_g->valid(node) && (*_node2label)[node]==select)
      return true;
  }
  return false;
}

template<typename GR,typename Con,typename BP, typename Option>
bool
  Solution_X<GR,Con,BP,Option>::isExist_t(unsigned select,RecordStore_Type& rs)
{
  std::string protein1 = rs.matchingEdges->getValue(0,select);
  std::string protein2 = rs.matchingEdges->getValue(1,select);
  if(_label2node->find(protein1)!=_label2node->end() &&
    _label2node->find(protein2)!=_label2node->end())
  {
    if(_label2node->at(protein1) == _label2node->at(protein2))
      return true;
  }
  return false;
}

template<typename GR,typename Con,typename BP, typename Option>
float
  Solution_X<GR,Con,BP,Option>::slow_discrepancy2(unsigned select,
  RecordStore_Type& rs,
  NetworkPool<GR,BP>& np,
  std::vector<Node>& overlappednodes,
  std::vector<unsigned>& overlappedids)
{
  float score=reCalculateScore2(rs);
  ///std::cout << "total score "<<score <<std::endl;
  for(int i=0; i<_numSpecies; i++)
  {
    std::string protein=rs._getRecords(select,i);
    //Label2NodeMap* label2Node=getLabel2Node(i);
    if(_label2node->find(protein)!=_label2node->end())
    {
      Node node=(*_label2node)[protein];
      if(!_g->valid(node))continue;      
      if(std::find(overlappednodes.begin(),overlappednodes.end(),node)==overlappednodes.end())
      {
        unsigned id=(*_node2label)[node];
        overlappedids.push_back(id);
        overlappednodes.push_back(node);  
      }
    }
  }
  updateState2(select,rs,np,overlappednodes);
  score=reCalculateScore2(rs)-score;
  return score;
}

template<typename GR,typename Con,typename BP, typename Option>
float
  Solution_X<GR,Con,BP,Option>::slow_discrepancy(unsigned select,
  RecordStore_Type& rs,
  NetworkPool<GR,BP>& np,
  std::vector<Node>& overlappednodes,
  std::vector<unsigned>& overlappedids)
{
  float score=reCalculateScore(rs);
  ///std::cout << "total score "<<score <<std::endl;
  for(unsigned i=0; i<NUM_SPECIES; i++)
  {
    std::string protein=rs.getRecords(select,i);
    Label2NodeMap* label2Node=getLabel2Node(i);
    if(label2Node->find(protein)!=label2Node->end())
    {
      Node node=(*label2Node)[protein];
      if(!_g->valid(node))continue;      
      if(std::find(overlappednodes.begin(),overlappednodes.end(),node)==overlappednodes.end())
      {
        unsigned id=(*_node2label)[node];
        overlappedids.push_back(id);
        overlappednodes.push_back(node);  
      }
    }
  }
  updateState(select,rs,np,overlappednodes);
  score=reCalculateScore(rs)-score;
  return score;
}

template<typename GR,typename Con,typename BP, typename Option>
void
  Solution_X<GR,Con,BP,Option>::recoverFromNew2(unsigned select,
  RecordStore_Type& rs,
  NetworkPool<GR,BP>& np,
  std::vector<unsigned>& overlappedids)
{
  removeNode2(select,rs);
  std::vector<unsigned>::iterator it;
  for(it=overlappedids.begin();it!=overlappedids.end();++it)
  {
    add2(*it,rs,np);
  }  
}

template<typename GR,typename Con,typename BP, typename Option>
void
  Solution_X<GR,Con,BP,Option>::recoverFromNew(unsigned select,
  RecordStore_Type& rs,
  NetworkPool<GR,BP>& np,
  std::vector<unsigned>& overlappedids)
{
  removeNode(select,rs);
  std::vector<unsigned>::iterator it;
  for(it=overlappedids.begin();it!=overlappedids.end();++it)
  {
    add(*it,rs,np);
  }  
}
template<typename GR,typename Con,typename BP, typename Option>
float
  Solution_X<GR,Con,BP,Option>::discrepancy(unsigned select,
  RecordStore_Type& rs,
  NetworkPool<GR,BP>& np,
  std::vector<Node>& overlappednodes)
{
  float nodescore=0.0;
  float edgescore=0.0;
  _delta.tozero();
  nodescore=rs.getRecordScore(select);
  for(unsigned i=0; i<_numSpecies; ++i)
  {
    std::string protein=rs.getRecords(select,i);
    Label2NodeMap* label2Node=getLabel2Node(i);
    GraphData *graph;
    graph = np.getGraph(i);
    if(label2Node->find(protein)!=label2Node->end())
    {
      Node node=(*label2Node)[protein];
      if(!_g->valid(node))continue;
      if(std::find(overlappednodes.begin(),overlappednodes.end(),node)==overlappednodes.end())
      {
        float temp=rs.getRecordScore((*_node2label)[node]);
        if(temp==_maxNodeScore)continue;
        overlappednodes.push_back(node);        
        _delta.deltaNodes-=temp;
        nodescore-=temp;
        for(IncEdgeIt e(*_g,node);e!=lemon::INVALID;++e)
        {
          if(!_g->valid(e))continue;
          _delta.deltaEdges-=(*_weightmap)[e];
          _delta.deltaNumEdge++;
          edgescore-=(*_weightmap)[e];
        }
      }
    }
    for(IncEdgeIt e(*(graph->g),(*(graph->invIdNodeMap))[protein]);
      e!=lemon::INVALID; ++e)//proteini might not exist in graph
    {
      Node neighbor=(graph->g)->runningNode(e);
      std::string pro=(*graph->label)[neighbor];
      if(protein==pro)continue;
      if(label2Node->find(pro)!=label2Node->end())
      {
        edgescore+=1.0/NUM_SPECIES;
      }
    }
  }
  float score;
  score=_alpha*(nodescore/_maxNodeScore)+(1-_alpha)*edgescore;
  return score;
}

template<typename GR,typename Con,typename BP, typename Option>
bool
  Solution_X<GR,Con,BP,Option>::_updateState(unsigned select,
  RecordStore_Type& rs,
  NetworkPool<GR,BP>& np,
  std::vector<Node>& overlappednodes)
{
  typename std::vector<Node>::iterator it;
  for(it=overlappednodes.begin();it!=overlappednodes.end();++it)
  {
    unsigned select=(*_node2label)[*it];
    for(unsigned i=0;i<NUM_SPECIES;i++)
    {
      _label2node->erase(rs._getRecords(select,i));
    }
    _node2label->set(*it,-1);
    _g->erase(*it);
    _numNode--;
    _allNodeScore-=rs._getRecordSeScore(select);
    _allStScore-=rs._getRecordStScore(select);
  }
  _reCalculateScore();
  return true;
}

template<typename GR,typename Con,typename BP, typename Option>
bool
  Solution_X<GR,Con,BP,Option>::updateState_t(unsigned select,RecordStore_Type& rs,float delta, int overlapValue)
{
  typedef std::unordered_multimap<int, std::string>::iterator Iterator;
  std::string protein1,protein2,protein_covered;
  protein1 = rs.matchingEdges->getValue(0,select);
  protein2 = rs.matchingEdges->getValue(1,select);
  if (overlapValue ==1)/// Conmbine protein2 to matchset 1 containing protein2
  {
    Node node1 = (*_label2node)[protein1];
    Node node2 = (*_label2node)[protein2];
    short species2 = rs.matchingEdges->getSpecies(protein2);
    (*_node2string)[node1]->insert(std::make_pair(species2,protein2));
    (*_label2node)[protein2] = node1;/// Revise Node information of protein2 
    auto range = (*_node2string)[node2]->equal_range(species2);
    for(Iterator it = range.first;it != range.second; it++)
    {
      if(it->second.compare(protein2)==0){
        (*_node2string)[node2]->erase(it);/// erase protein2 from matchset 2
        break;}
    }
  }
  else if (overlapValue==2)/// Substitute protein_covered by protein 2
  {
    Node node1 = (*_label2node)[protein1];
    Node node2 = (*_label2node)[protein2];
    short species2 = rs.matchingEdges->getSpecies(protein2);
    auto range = (*_node2string)[node1]->equal_range(species2);
    Iterator it = range.first;
    protein_covered=it->second;
    it->second = protein2; /// substitute the first protein with protein2 in matchset 1
    (*_label2node)[protein2] = node1;/// Revise Node information of protein2 
    range = (*_node2string)[node2]->equal_range(species2);
    for(Iterator it = range.first;it != range.second; it++)
    {
      if(it->second.compare(protein2)==0){
        (*_node2string)[node2]->erase(it);/// erase protein2 from matchset 2
        break;}
    }
  }
  else if (overlapValue==3)/// Combine protein 1 to matchset 2
  {
    Node node1 = (*_label2node)[protein1];
    Node node2 = (*_label2node)[protein2];
    short species1 = rs.matchingEdges->getSpecies(protein1);
    (*_node2string)[node2]->insert(std::make_pair(species1,protein1));
    (*_label2node)[protein1] = node2;/// Revise Node information of protein1

    auto range = (*_node2string)[node1]->equal_range(species1);
    for(Iterator it = range.first;it != range.second; it++)
    {
      if(it->second.compare(protein1)==0){
        (*_node2string)[node1]->erase(it);/// erase protein1 from matchset 1
        break;}
    }
  }
  else if (overlapValue==4)/// Subsititute protein_covered by protein 1.
  {
	Node node1 = (*_label2node)[protein1];
    Node node2 = (*_label2node)[protein2];
    short species1 = rs.matchingEdges->getSpecies(protein1);
    auto range = (*_node2string)[node2]->equal_range(species1);
    Iterator it = range.first;
    protein_covered=it->second;
    it->second = protein1; /// Substitute the first protein with protein1 in matchset 2.
    (*_label2node)[protein1] = node2;/// Revise Node information of protein1.
    range = (*_node2string)[node1]->equal_range(species1);
    for(Iterator it = range.first;it != range.second; it++)
    {
      if(it->second.compare(protein1)==0){
        (*_node2string)[node1]->erase(it);/// Erase protein1 from matchset 1
        break;}
    }
  }
  else if (overlapValue==5)/// Combine protein2 to matchset 1
  {
	Node node1 = (*_label2node)[protein1];
    short species2 = rs.matchingEdges->getSpecies(protein2);
    (*_node2string)[node1]->insert(std::make_pair(species2,protein2));
    (*_label2node)[protein2] = node1;
  }else if (overlapValue==6)/// Substitute protein_covered by protein 2
  {
	Node node1 = (*_label2node)[protein1];
    short species2 = rs.matchingEdges->getSpecies(protein2);
    auto range = (*_node2string)[node1]->equal_range(species2);
    Iterator it = range.first;
    protein_covered=it->second;
    it->second = protein2; /// substitute the first protein with protein2 in matchset 1
    (*_label2node)[protein2] = node1;
  }else if (overlapValue==7)/// Combine protein1 to matchset 2
  {
    Node node2 = (*_label2node)[protein2];
    short species1 = rs.matchingEdges->getSpecies(protein1);
    (*_node2string)[node2]->insert(std::make_pair(species1,protein1));
    (*_label2node)[protein1] = node2;/// Revise Node information of protein1
  }else if (overlapValue==8)/// Substitute protein_covered by protein 1
  {
    Node node2 = (*_label2node)[protein2];
    short species1 = rs.matchingEdges->getSpecies(protein1);
    auto range = (*_node2string)[node2]->equal_range(species1);
    Iterator it = range.first;
    protein_covered=it->second;
    it->second = protein1; /// Substitute the first protein with protein1 in matchset 2.
    (*_label2node)[protein1] = node2;/// Revise Node information of protein1.
  }
  if(!protein_covered.compare(""))
	_label2node->erase(protein_covered);
  _alignmentScore+=delta;
  return true;
}

template<typename GR,typename Con,typename BP, typename Option>
bool
  Solution_X<GR,Con,BP,Option>::updateState2(unsigned select,
  RecordStore_Type& rs,
  NetworkPool<GR,BP>& np,
  std::vector<Node>& overlappednodes)
{
  typename std::vector<Node>::iterator it;
  for(it=overlappednodes.begin();it!=overlappednodes.end();++it)
  {
    unsigned select=(*_node2label)[*it];
    for(unsigned i=0;i<NUM_SPECIES;i++)
    {
      _label2node->erase(rs._getRecords(select,i));
    }
    _node2label->set(*it,-1);
    _g->erase(*it);
  }
  add2(select,rs,np);
  return true;
}

template<typename GR,typename Con,typename BP, typename Option>
bool
  Solution_X<GR,Con,BP,Option>::updateState(unsigned select,
  RecordStore_Type& rs,
  NetworkPool<GR,BP>& np,
  std::vector<Node>& overlappednodes)
{
  typename std::vector<Node>::iterator it;
  for(it=overlappednodes.begin();it!=overlappednodes.end();++it)
  {
    unsigned nodeid=(*_node2label)[*it];
    _node2label->set(*it,-1);
    _label2node1->erase(rs.getRecords(nodeid,0));
    _label2node2->erase(rs.getRecords(nodeid,1));
    _label2node3->erase(rs.getRecords(nodeid,2));
    _g->erase(*it);
    _numNode--;
  }
  add(select,rs,np);
  return true;
}

template<typename GR,typename Con,typename BP, typename Option>
void
  Solution_X<GR,Con,BP,Option>::removeNode2(unsigned id,RecordStore_Type& rs){
    Node node;
    node=(*_label2node)[rs._getRecords(id,0)];
    _node2label->set(node,-1);
    for(int i=0;i<_numSpecies;i++)
    {
      _label2node->erase(rs._getRecords(id,i));
    }
    _g->erase(node);
}

template<typename GR,typename Con,typename BP, typename Option>
void
  Solution_X<GR,Con,BP,Option>::removeNode(unsigned id,RecordStore_Type& rs){
    Node node;
    node=(*_label2node1)[rs.getRecords(id,0)];
    _node2label->set(node,-1);
    _label2node1->erase(rs.getRecords(id,0));
    _label2node2->erase(rs.getRecords(id,1));
    _label2node3->erase(rs.getRecords(id,2));
    _g->erase(node);
    _numNode--;
}
template<typename GR,typename Con,typename BP, typename Option>
GR&
  Solution_X<GR,Con,BP,Option>::getGraph()
{
  return *_g; 
}

template<typename GR,typename Con,typename BP, typename Option>
bool
  Solution_X<GR,Con,BP,Option>::validNode(Node& n)
{
  return _g->valid(n);
}

template<typename GR,typename Con,typename BP, typename Option>
unsigned
  Solution_X<GR,Con,BP,Option>::getNodeId(Node& n)
{
  return (*_node2label)[n];
}

template<typename GR,typename Con,typename BP, typename Option>
std::unordered_multimap<int,std::string>*
  Solution_X<GR,Con,BP,Option>::getNodeLabel(Node& n)
{
  return (*_node2string)[n];
}

template<typename GR,typename Con,typename BP, typename Option>
void
  Solution_X<GR,Con,BP,Option>::getConservedEdges()
{
  for(EdgeIt e(*_g);e!=lemon::INVALID;++e)
  {
    if(!_g->valid(e))continue;
    int myindex = floor((*_weightmap)[e]*_numSpecies);
    _numConserved[myindex]++;
  }
}

template<typename GR,typename Con,typename BP, typename Option>
void
  Solution_X<GR,Con,BP,Option>::getConservedEdges_t(NetworkPool_Type& networkpool)
{
  //typename NetworkPool::GraphData* networks;
  for(NodeIt node1(*_g); node1!=lemon::INVALID; ++node1)
  {

    for(NodeIt node2(*_g); node2!=lemon::INVALID; ++node2)
    {
      if(node2<node1){
        int  myindex = 0;
        bool isConserved = false;
        for(int i=0;i<_numSpecies;++i)
        {
          typename NetworkPool_Type::GraphData* network;
          network = networkpool.getGraph(i);
          std::string protein1,protein2;
          protein1 = (*_node2string)[node1][i];
          protein2 = (*_node2string)[node2][i];
          if(network->invIdNodeMap->find(protein1)==network->invIdNodeMap->end())continue;
          if(network->invIdNodeMap->find(protein2)==network->invIdNodeMap->end())continue;
          Node netnode1 = (*network->invIdNodeMap)[protein1];
          for(IncEdgeIt e((*network->g),netnode1); e!=lemon::INVALID; ++e)
          {
            Node netnode2 = network->g->runningNode(e);
            if((*network->label)[netnode2].compare(protein2)==0)
            {
              if(!isConserved)
              {
                _g->addEdge(node1,node2);
                isConserved = true;
              }
              myindex++;
              break;
            }
          }
        }
        _numConserved[myindex]++;
      }
    }
  }
}

template<typename GR,typename Con,typename BP, typename Option>
void
  Solution_X<GR,Con,BP,Option>::printDecomposedScore()
{
  //std::cerr << "Nodes score:" << _alpha*(_allNodeScore/_maxNodeScore)<<std::endl;
  //std::cerr << "Edges score:" << (1-_alpha)*_allEdgeScore<<std::endl;
}
#endif //SOLUTION_X_H_
