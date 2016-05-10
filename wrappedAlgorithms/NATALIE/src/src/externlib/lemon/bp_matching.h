/* -*- mode: C++; indent-tabs-mode: nil; -*-
 *
 * This file is a part of LEMON, a generic C++ optimization library.
 *
 * Copyright (C) 2003-2010
 * Egervary Jeno Kombinatorikus Optimalizalasi Kutatocsoport
 * (Egervary Research Group on Combinatorial Optimization, EGRES).
 *
 * Permission to use, modify and distribute this software is granted
 * provided that this copyright notice appears in all copies. For
 * precise terms see the accompanying LICENSE file.
 *
 * This software is provided "AS IS" with no warranty of any kind,
 * express or implied, and with no claim as to its suitability for any
 * purpose.
 *
 */

#ifndef BP_MATCHING_H
#define BP_MATCHING_H

#include <limits>
#include <list>
#include <algorithm>
#include <assert.h>
#include <queue>
#include <math.h>

#include <lemon/core.h>
#include <lemon/bin_heap.h>

///\ingroup matching
///\file
///\brief Maximum weight matching algorithms in bipartite graphs.

namespace lemon {

  /// \ingroup matching
  ///
  /// \brief Maximum weight matching in (sparse) bipartite graphs
  ///
  /// This class implements a successive shortest path algorithm for finding
  /// a maximum weight matching in an undirected bipartite graph.
  /// Let \f$G = (X \cup Y, E)\f$ be an undirected bipartite graph. The
  /// following linear program corresponds to a maximum weight matching
  /// in the graph \f$G\f$.
  ///
  /** \f$\begin{array}{rrcll} \
      \max & \displaystyle\sum_{(i,j) \in E} c_{ij} x_{ij}\\ \
      \mbox{s.t.} & \displaystyle\sum_{i \in X} x_{ij} & \leq & 1, \
      & \forall j \in \{ j^\prime \in Y \mid (i,j^\prime) \in E \}\\ \
      & \displaystyle\sum_{j \in Y} x_{ij} & \leq & 1, \
      & \forall i \in \{ i^\prime \in X \mid (i^\prime,j) \in E \}\\ \
      & x_{ij}        & \geq & 0, & \forall (i,j) \in E\\\end{array}\f$
  */
  ///
  /// where \f$c_{ij}\f$ is the weight of edge \f$(i,j)\f$. The dual problem
  /// is:
  ///
  /** \f$\begin{array}{rrcll}\min & \displaystyle\sum_{v \in X \cup Y} p_v\\ \
      \mbox{s.t.} & p_i + p_j & \geq & c_{ij}, & \forall (i,j) \in E\\ \
      & p_v & \geq & 0, & \forall v \in X \cup Y \end{array}\f$
  */
  ///
  /// A maximum weight matching is constructed by iteratively considering the
  /// vertices in \f$X = \{x_1, \ldots, x_n\}\f$. In every iteration \f$k\f$
  /// we establish primal and dual complementary slackness for the subgraph
  /// \f$G[X_k \cup Y]\f$ where \f$X_k = \{x_1, \ldots, x_k\}\f$.
  /// So after the final iteration the primal and dual solution will be equal,
  /// and we will thus have a maximum weight matching. The time complexity of
  /// this method is \f$O(n(n + m)\log n)\f$.
  ///
  /// In case the bipartite graph is dense, it is better to use
  /// \ref MaxWeightedDenseBipartiteMatching, which has a time complexity of
  /// \f$O(n^3)\f$.
  ///
  /// \tparam BGR The bipartite graph type the algorithm runs on.
  /// \tparam WM The type edge weight map. The default type is
  /// \ref concepts::Graph::EdgeMap "BGR::EdgeMap<int>".
#ifdef DOXYGEN
  template <typename BGR, typename WM>
#else
  template <typename BGR,
            typename WM = typename BGR::template EdgeMap<int> >
#endif
  class MaxWeightedBipartiteMatching
  {
  public:
    /// The graph type of the algorithm
    typedef BGR BpGraph;
    /// The type of the edge weight map
    typedef WM WeightMap;
  
  private:
    TEMPLATE_BPGRAPH_TYPEDEFS(BpGraph);
  
  public:
    /// The value type of the edge weights
    typedef typename WeightMap::Value Value;
    /// The type of the matching map
    typedef typename BpGraph::
      template NodeMap<typename BpGraph::Edge> MatchingMap;
    /// The type of the potential node map
    typedef typename BpGraph::template NodeMap<Value> PotMap;
    /// The type of the distance node map
    typedef typename BpGraph::template NodeMap<Value> DistMap;
    /// The type of the predecessor map
    typedef typename BpGraph::template NodeMap<Arc> PredMap;
    /// The type of the heap cross reference
    typedef typename BpGraph::template BlueNodeMap<int> HeapCrossRef;
    /// The type of the heap
    typedef BinHeap<Value, HeapCrossRef> Heap;

  private:
    typedef std::list<RedNode> RedNodeList;
    typedef std::list<BlueNode> BlueNodeList;

    const BpGraph& _graph;
    const WeightMap& _weight;

    PotMap* _pPot;
    bool _localPot;
    MatchingMap* _pMatchingMap;
    bool _localMatchingMap;
    DistMap* _pDist;
    bool _localDist;
    PredMap* _pPred;
    bool _localPred;
    HeapCrossRef* _pHeapCrossRef;
    bool _localHeapCrossRef;
    Heap* _pHeap;
    bool _localHeap;

    Value _matchingWeight;
    int _matchingSize;

    static const Value _maxValue;

    void createStructures()
    {
      if (!_pPot)
      {
        _pPot = new PotMap(_graph, 0);
        _localPot = true;
      }
      if (!_pMatchingMap)
      {
        _pMatchingMap = new MatchingMap(_graph, INVALID);
        _localMatchingMap = true;
      }
      if (!_pDist)
      {
        _pDist = new DistMap(_graph, _maxValue);
        _localDist = true;
      }
      if (!_pPred)
      {
        _pPred = new PredMap(_graph, INVALID);
        _localPred = true;
      }
      if (!_pHeapCrossRef)
      {
        _pHeapCrossRef = new HeapCrossRef(_graph, Heap::PRE_HEAP);
        _localHeapCrossRef = true;
      }
      if (!_pHeap)
      {
        _pHeap = new Heap(*_pHeapCrossRef);
        _localHeap = true;
      }
    }

    void destroyStructures()
    {
      if (_localPot)
        delete _pPot;
      if (_localMatchingMap)
        delete _pMatchingMap;
      if (_localDist)
        delete _pDist;
      if (_localPred)
        delete _pPred;
      if (_localHeapCrossRef)
        delete _pHeapCrossRef;
      if (_localHeap)
        delete _pHeap;
    }

    bool isFree(const Node& v)
    {
      return (*_pMatchingMap)[v] == INVALID;
    }

    void augmentPath(Arc a, bool matching)
    {
      // M' = M ^ EP
      while (a != INVALID)
      {
        if (!matching)
        {
          _pMatchingMap->set(_graph.source(a), a);
          _pMatchingMap->set(_graph.target(a), a);
        }

        matching = !matching;
        a = (*_pPred)[_graph.source(a)];
      }
    }

    void augment(const Node& x)
    {
      assert(isFree(x));
      assert(_pHeap->empty());

      /**
       * In case maxCardinality == false, we also need to consider
       * augmenting paths starting from x and ending in a matched
       * node x' in X. Augmenting such a path does *not* increase
       * the cardinality of the matching. It may, however, increase
       * the weight of the matching.
       *
       * Along with a shortest path starting from x and ending in
       * a free vertex y in Y, we also determine x' such that
       *   y' = pred[x'],
       *   (pot[x] + pot[y'] - dist[x, y']) - w(y', x') is maximal
       *
       * Since (y', x') is part of the matching,
       * by primal complementary slackness we have that
       *   pot[y'] + pot[x'] = w(y', x').
       *
       * Hence
       * x' = arg max_{x' \in X} { pot[x] + pot[y'] - dist[x, y']) -w(y', x') }
       *    = arg max_{x' \in X} { pot[x] - dist[x, y'] - pot[x'] }
       *    = arg max_{x' \in X} { -dist[x, y'] - pot[x'] }
       *    = arg min_{x' \in X} { dist[x, y'] + [x'] }
       *
       * We only augment x ->* x' if dist(x,y) > dist[x, y'] + pot[x']
       * Otherwise we augment x ->* y.
       */

      Value UB = (*_pPot)[x];
      _pDist->set(x, 0);
      _pPred->set(x, INVALID);

      RedNodeList visitedX;
      BlueNodeList visitedY;

      // add nodes adjacent to x to heap, and update UB
      visitedX.push_back(_graph.asRedNodeUnsafe(x));
      for (IncEdgeIt e(_graph, x); e != INVALID; ++e)
      {
        const BlueNode y = _graph.blueNode(e);
        
        assert((*_pDist)[y] == _maxValue);
        Value dist_y = (*_pPot)[x] + (*_pPot)[y] - _weight[e];

        if (dist_y >= UB)
          continue;

        if (isFree(y))
          UB = dist_y;

        _pDist->set(y, dist_y);
        _pPred->set(y, _graph.direct(e, x));

        assert(_pHeap->state(y) == Heap::PRE_HEAP);
        _pHeap->push(y, dist_y);
      }

      Node x_min = x;
      Value min_dist = 0, x_min_dist = (*_pPot)[x];

      while (true)
      {
        assert(_pHeap->empty() || _pHeap->prio() == (*_pDist)[_pHeap->top()]);

        if (_pHeap->empty() || _pHeap->prio() >= x_min_dist)
        {
          min_dist = x_min_dist;

          if (x_min != x)
          {
            // we have an augmenting path between x and x_min
            // that doesn't increase the matching size
            _matchingWeight += (*_pPot)[x] - x_min_dist;

            // x_min becomes free, and will always remain free
            _pMatchingMap->set(x_min, INVALID);
            augmentPath((*_pPred)[x_min], true);
          }
          break;
        }

        const BlueNode y = _pHeap->top();
        const Value dist_y = _pHeap->prio();
        _pHeap->pop();
        _pHeap->state(y, Heap::PRE_HEAP);

        visitedY.push_back(y);
        if (isFree(y))
        {
          // we have an augmenting path between x and y
          augmentPath((*_pPred)[y], false);
          _matchingSize++;

          assert((*_pPot)[y] == 0);
          _matchingWeight += (*_pPot)[x] - dist_y;

          min_dist = dist_y;
          break;
        }
        else
        {
          // y is not free, so there *must* be only one arc pointing toward X
          const Edge e = (*_pMatchingMap)[y];
          assert(_graph.blueNode(e) == y);

          const RedNode x2 = _graph.redNode(e);
          _pPred->set(x2, _graph.direct(e, y));
          visitedX.push_back(x2);
          _pDist->set(x2, dist_y); // matched edges have a reduced weight of 0

          if (dist_y + (*_pPot)[x2] < x_min_dist)
          {
            x_min = x2;
            x_min_dist = dist_y + (*_pPot)[x2];

            // we have a better criterion now
            if (UB > x_min_dist)
              UB = x_min_dist;
          }

          for (IncEdgeIt e2(_graph, x2); e2 != INVALID; ++e2)
          {
            if (static_cast<const Edge>(e2) == e) continue;

            const BlueNode y2 = _graph.blueNode(e2);

            Value dist_y2 = dist_y + (*_pPot)[x2] + (*_pPot)[y2] - _weight[e2];

            if (dist_y2 >= UB)
              continue;

            if (isFree(y2))
              UB = dist_y2;

            //if (_pHeap->state(y2) == Heap::PRE_HEAP)
            //{
            //  dist[y2] = dist_y2;
            //  pred[y2] = _graph.direct(e2, x2);
            //  _pHeap->push(y2, dist_y2);
            //}
            //else 
            if (dist_y2 < (*_pDist)[y2])
            {
              //assert(_pHeap->state(y2) == Heap::IN_HEAP);
              _pDist->set(y2, dist_y2);
              _pPred->set(y2, _graph.direct(e2, x2));
              _pHeap->set(y2, dist_y2);
            }
          }
        }
      }

      // restore primal and dual complementary slackness
      for (typename RedNodeList::const_iterator itX = visitedX.begin();
        itX != visitedX.end(); itX++)
      {
        const RedNode& x = *itX;
        assert(min_dist - (*_pDist)[x] >= 0);
        (*_pPot)[x] -= min_dist - (*_pDist)[x];
        assert((*_pPot)[x] >= 0);

        // reset distance to INF
        _pDist->set(x, _maxValue);
      }

      for (typename BlueNodeList::const_iterator itY = visitedY.begin();
        itY != visitedY.end(); itY++)
      {
        const BlueNode& y = *itY;
        assert(min_dist - (*_pDist)[y] >= 0);
        (*_pPot)[y] += min_dist - (*_pDist)[y];
        assert((*_pPot)[y] >= 0);

        // reset distance to INF
        _pDist->set(y, _maxValue);
      }

      // clear the heap, and reset distances
      while (!_pHeap->empty())
      {
        const BlueNode y = _pHeap->top();
        _pHeap->pop();
        _pDist->set(y, _maxValue);
        _pHeap->state(y, Heap::PRE_HEAP);
      }
    }

  public:
    /// \brief Constructor
    ///
    /// Constructor.
    ///
    /// \param graph is the input graph
    /// \param weight are the edge weights
    MaxWeightedBipartiteMatching(const BpGraph& graph, const WeightMap& weight)
      : _graph(graph)
      , _weight(weight)
      , _pPot(NULL)
      , _localPot(false)
      , _pMatchingMap(NULL)
      , _localMatchingMap(false)
      , _pDist(NULL)
      , _localDist(false)
      , _pPred(NULL)
      , _localPred(false)
      , _pHeapCrossRef(NULL)
      , _localHeapCrossRef(false)
      , _pHeap(NULL)
      , _localHeap(false)
      , _matchingWeight(0)
      , _matchingSize(0)
    {
    }

    ~MaxWeightedBipartiteMatching()
    {
      destroyStructures();
    }

    /// \brief Sets the map that stores the node potentials
    ///
    /// Sets the map that stores the nodes calculated by the algorithm.
    /// If you don't use this function before calling \ref run()
    /// or \ref init(), an instance will be allocated automatically.
    /// The destructor deallocates this automatically allocated map,
    /// of course.
    /// \return <tt> (*this) </tt>
    MaxWeightedBipartiteMatching& potMap(PotMap& m)
    {
      if (_localPot)
      {
        delete _pPot;
        _localPot = false;
      }
      _pPot = &m;
      return *this;
    }

    /// \brief Sets the map that stores the matching edge incident 
    /// to each node
    ///
    /// Sets the map that stores the matching edge incident to each node
    /// calculated by the algorithm.
    /// If you don't use this function before calling \ref run()
    /// or \ref init(), an instance will be allocated automatically.
    /// The destructor deallocates this automatically allocated map,
    /// of course.
    /// \return <tt> (*this) </tt>
    MaxWeightedBipartiteMatching& matchingMap(MatchingMap& m)
    {
      if (_localMatchingMap)
      {
        delete _pMatchingMap;
        _localMatchingMap = false;
      }
      _pMatchingMap = &m;
      return *this;
    }

    /// \brief Sets the map that stores the distances of the nodes
    ///
    /// Sets the map that stores the distances of the nodes calculated by the
    /// algorithm.
    /// If you don't use this function before calling \ref run()
    /// or \ref init(), an instance will be allocated automatically.
    /// The destructor deallocates this automatically allocated map,
    /// of course.
    /// \return <tt> (*this) </tt>
    MaxWeightedBipartiteMatching& distMap(DistMap &m)
    {
      if (_localDist)
      {
        delete _pDist;
        _localDist = false;
      }
      _pDist = &m;
      return *this;
    }

    /// \brief Sets the map that stores the predecessor arcs
    ///
    /// Sets the map that stores the predecessor arcs.
    /// If you don't use this function before calling \ref run()
    /// or \ref init(), an instance will be allocated automatically.
    /// The destructor deallocates this automatically allocated map,
    /// of course.
    /// \return <tt> (*this) </tt>
    MaxWeightedBipartiteMatching& predMap(PredMap& m)
    {
      if (_localPred)
      {
        delete _pPred;
        _localPred = false;
      }
      _pPred = &m;
      return *this;
    }

    /// Sets the heap and the cross reference used by algorithm
    ///
    /// Sets the heap and the cross reference used by algorithm
    /// If you don't use this function before calling \ref run()
    /// or \ref init(), heap and cross reference instances will be
    /// allocated automatically.
    /// The destructor deallocates these automatically allocated objects,
    /// of course.
    /// \return <tt> (*this) </tt>
    MaxWeightedBipartiteMatching& heap(Heap& hp, HeapCrossRef& cr)
    {
      if (_localHeapCrossRef)
      {
        delete _pHeapCrossRef;
        _localHeapCrossRef = false;
      }
      _pHeapCrossRef = &cr;
      if(_localHeap)
      {
        delete _pHeap;
        _localHeap = false;
      }
      _pHeap = &hp;
      return *this;
    }

    /// \brief Initialize the algorithm
    ///
    /// This function initializes the algorithm.
    ///
    /// \param greedy indicates whether a nonempty initial matching
    /// should be used; this might be faster in some cases.
    void init(bool greedy = true)
    {
      createStructures();

      _matchingWeight = 0;
      _matchingSize = 0;

      // pot[y] is set to 0
      for (BlueNodeIt y(_graph); y != INVALID; ++y)
      {
        assert((*_pDist)[y] == _maxValue);
        assert((*_pHeapCrossRef)[y] == Heap::PRE_HEAP);
        _pPot->set(y, 0);
      }

      // pot[x] is set to maximum incident edge weight
      for (RedNodeIt x(_graph); x != INVALID; ++x)
      {
        assert((*_pDist)[x] == _maxValue);

        Value max_weight = 0;
        Edge e_max = INVALID;
        for (IncEdgeIt e(_graph, x); e != INVALID; ++e)
        {
          if (_weight[e] > max_weight)
          {
            max_weight = _weight[e];
            e_max = e;
          }
        }

        if (e_max != INVALID)
        {
          _pPot->set(x, max_weight);

          const Node y = _graph.blueNode(e_max);
          if (greedy && isFree(y))
          {
            _matchingWeight += max_weight;
            _matchingSize++;
            _pMatchingMap->set(x, e_max);
            _pMatchingMap->set(y, e_max);
          }
        }
      }
    }

    /// \brief Start the algorithm
    ///
    /// This function starts the algorithm.
    ///
    /// \pre \ref init() must have been called before using this function.
    void start()
    {
      for (RedNodeIt x(_graph); x != INVALID; ++x)
      {
        if (isFree(x))
          augment(x);
      }
    }

    /// \brief Run the algorithm.
    ///
    /// This method runs the \c %MaxWeightedBipartiteMatching algorithm.
    ///
    /// \param greedy indicates whether a nonempty initial matching
    /// should be used; this might be faster in some cases.
    ///
    /// \note mwbm.run() is just a shortcut of the following code.
    /// \code
    ///   mwbm.init();
    ///   mwbm.start();
    /// \endcode
    void run(bool greedy = true)
    {
      init(greedy);
      start();
    }

    /// \brief Check whether the solution is optimal
    ///
    /// Check using the dual solution whether the primal solution is optimal.
    ///
    /// \return \c true if the solution is optimal.
    bool checkOptimality() const
    {
      assert(_pMatchingMap && _pPot);

      /*
       * Primal:
       *   max  \sum_{i,j} c_{ij} x_{ij}
       *   s.t. \sum_i x_{ij} <= 1
       *        \sum_j x_{ij} <= 1
       *               x_{ij} >= 0
       *
       * Dual:
       *   min  \sum_j p_j + \sum_i r_i
       *   s.t. p_j + r_i >= c_{ij}
       *              p_j >= 0
       *              r_i >= 0
       *
       * Solution is optimal iff:
       * - Primal complementary slackness:
       *   - x_{ij} = 1  =>  p_j + r_i = c_{ij}
       * - Dual complementary slackness:
       *   - p_j != 0    =>  \sum_i x_{ij} = 1
       *   - r_i != 0    =>  \sum_j x_{ij} = 1
       */

      // check primal solution
      for (NodeIt n(_graph); n != INVALID; ++n)
      {
        const Edge e = (*_pMatchingMap)[n];

        if (e != INVALID)
        {
          const Node u = _graph.u(e);
          const Node v = _graph.v(e);

          if (n != u && n != v)
            return false; // e must be incident to n
          if ((*_pMatchingMap)[u] != (*_pMatchingMap)[v])
            return false; // primal feasibility
          if ((*_pPot)[u] + (*_pPot)[v] != _weight[e])
            return false; // primal complementary slackness
        }
      }

      // check dual solution
      for (NodeIt n(_graph); n != INVALID; ++n)
      {
        const Value pot_n = (*_pPot)[n];
        if (pot_n < 0)
          return false; // dual feasibility
        if ((*_pMatchingMap)[n] == INVALID && pot_n != 0)
          return false; // dual complementary slackness
      }
      for (EdgeIt e(_graph); e != INVALID; ++e)
      {
        if ((*_pPot)[_graph.u(e)] + (*_pPot)[_graph.v(e)] < _weight[e])
          return false; // dual feasibility
      }

      return true;
    }

    /// \brief Return the dual value of the given node
    ///
    /// This function returns the potential of the given node
    ///
    /// \pre init() must have been called before using this function
    const Value pot(const Node& n) const
    {
      assert(_pPot);
      return (*_pPot)[n];
    }

    /// \brief Return a const reference to the matching map.
    ///
    /// This function returns a const reference to a node map that stores
    /// the matching edge incident to each node.
    ///
    /// \pre init() must have been called before using this function.
    const MatchingMap& matchingMap() const
    {
      assert(_pMatchingMap);
      return *_pMatchingMap;
    }

    /// \brief Return the weight of the matching.
    ///
    /// This function returns the weight of the found matching.
    ///
    /// \pre init() must have been called before using this function.
    Value matchingWeight() const
    {
      return _matchingWeight;
    }

    /// \brief Return the number of edges in the matching.
    ///
    /// This function returns the number of edges in the matching.
    int matchingSize() const
    {
      return _matchingSize;
    }

    /// \brief Return \c true if the given edge is in the matching.
    ///
    /// This function returns \c true if the given edge is in the found
    /// matching.
    ///
    /// \pre init() must have been been called before using this function.
    bool matching(const Edge& e) const
    {
      assert(_pMatchingMap);
      return e != INVALID && (*_pMatchingMap)[_graph.u(e)] != INVALID;
    }

    /// \brief Return the matching edge incident to the given node.
    ///
    /// This function returns the matching edge incident to the
    /// given node in the found matching or \c INVALID if the node is
    /// not covered by the matching.
    ///
    /// \pre init() must have been been called before using this function.
    Edge matching(const Node& n) const
    {
      assert(_pMatchingMap);
      return (*_pMatchingMap)[n];
    }

    /// \brief Return the mate of the given node.
    ///
    /// This function returns the mate of the given node in the found
    /// matching or \c INVALID if the node is not covered by the matching.
    ///
    /// \pre init() must have been been called before using this function.
    Node mate(const Node& n) const
    {
      assert(_pMatchingMap);

      const Edge e = (*_pMatchingMap)[n];

      if (e == INVALID)
        return INVALID;
      else
        return _graph.oppositeNode(n, e);
    }

    static void scale(const BGR& graph, 
                      typename BGR::template EdgeMap<double>& weight)
    {
      double maxWeight = 0;
      for (typename BGR::EdgeIt e(graph); e != INVALID; ++e)
      {
        double w = fabs(weight[e]);
        if (w > maxWeight)
          maxWeight = w;
      }

      scale(graph, weight, maxWeight);
    }

    static void scale(const BGR& graph,
                      const typename BGR::template EdgeMap<double>& inWeight,
                      typename BGR::template EdgeMap<double>& outWeight)
    {
      double maxWeight = 0;
      for (typename BGR::EdgeIt e(graph); e != INVALID; ++e)
      {
        double w = fabs(inWeight[e]);
        if (w > maxWeight)
          maxWeight = w;
      }

      scale(graph, inWeight, outWeight, maxWeight);
    }

    static void scale(const BGR& graph, 
                      typename BGR::template EdgeMap<double>& weight, 
                      double maxWeight)
    {
      int exp;
      frexp(3 * maxWeight, &exp);
      double one_over_S = ldexp(1, exp - 53);
      double S = ldexp(1, 53 - exp);

      for (typename BGR::EdgeIt e(graph); e != INVALID; ++e)
      {
        double w = weight[e];
        if (w != 0)
        {
          int sign = 1;
          if (w < 0)
          {
            sign = -1;
            w = -w;
          }

          weight[e] = sign * floor(w * S) * one_over_S;
        }
      }
    }

    static void scale(const BGR& graph,
                      const typename BGR::template EdgeMap<double>& inWeight,
                      typename BGR::template EdgeMap<double>& outWeight,
                      double maxWeight)
    {
      int exp;
      frexp(3 * maxWeight, &exp);
      double one_over_S = ldexp(1., exp - 53);
      double S = ldexp(1., 53 - exp);

      for (typename BGR::EdgeIt e(graph); e != INVALID; ++e)
      {
        double w = inWeight[e];
        if (w != 0)
        {
          int sign = 1;
          if (w < 0)
          {
            sign = -1;
            w = -w;
          }

          outWeight[e] = sign * floor(w * S) * one_over_S;
        }
        else
        {
          // outWeight[e] need not be 0
          outWeight[e] = 0;
        }
      }
    }

    static double scale(double w, double S, double one_over_S)
    {
      if (w != 0)
      {
        int sign = 1;
        if (w < 0)
        {
          sign = -1;
          w = -w;
        }

        return sign * floor(w * S) * one_over_S;
      }
      else
      {
        return 0;
      }
    }

    static void scale_get_factors(double maxWeight, double& S, double& one_over_S)
    {
      int exp;
      frexp(3 * maxWeight, &exp);
      one_over_S = ldexp(1., exp - 53);
      S = ldexp(1., 53 - exp);
    }
  };

  template<typename BGR, typename WM>
  const typename WM::Value MaxWeightedBipartiteMatching<BGR, WM>::
    _maxValue = std::numeric_limits<typename WM::Value>::max();

  /// \ingroup matching
  ///
  /// \brief Maximum weight matching in (dense) bipartite graphs
  ///
  /// This class provides an implementation of the classical Hungarian
  /// algorithm for finding a maximum weight matching in an undirected
  /// bipartite graph. This algorithm follows the primal-dual schema.
  /// The time complexity is \f$O(n^3)\f$. In case the bipartite graph is
  /// sparse, it is better to use \ref MaxWeightedBipartiteMatching, which
  /// has a time complexity of \f$O(n^2 \log n)\f$ for sparse graphs.
  ///
  /// \tparam BGR The bipartite graph type the algorithm runs on.
  /// \tparam WM The type edge weight map. The default type is
  /// \ref concepts::Graph::EdgeMap "BGR::EdgeMap<int>".
#ifdef DOXYGEN
  template <typename BGR, typename WM>
#else
  template <typename BGR,
            typename WM = typename BGR::template EdgeMap<int> >
#endif
  class MaxWeightedDenseBipartiteMatching
  {
  public:
    /// The graph type of the algorithm
    typedef BGR BpGraph;
    /// The type of the edge weight map
    typedef WM WeightMap;
    /// The value type of the edge weights
    typedef typename WeightMap::Value Value;
    /// The type of the matching map
    typedef typename BpGraph::
      template NodeMap<typename BpGraph::Edge> MatchingMap;

  private:
    TEMPLATE_BPGRAPH_TYPEDEFS(BpGraph);

    typedef typename BpGraph::template NodeMap<int> IdMap;
    typedef std::vector<int> MateVector;
    typedef std::vector<Value> WeightVector;
    typedef std::vector<bool> BoolVector;

    class BpEdgeT
    {
    private:
      Value _weight;
      Edge _edge;

    public:
      BpEdgeT()
        : _weight(0)
        , _edge(INVALID)
      {
      }

      void setWeight(Value weight)
      {
        _weight = weight;
      }

      Value getWeight() const
      {
        return _weight;
      }

      void setEdge(const Edge& edge)
      {
        _edge = edge;
      }

      const Edge& getEdge() const
      {
        return _edge;
      }
    };

    typedef std::vector<std::vector<BpEdgeT> > AdjacencyMatrixType;

    const BpGraph& _graph;
    const WeightMap& _weight;
    IdMap _idMap;
    MatchingMap _matchingMap;

    AdjacencyMatrixType _adjacencyMatrix;
    WeightVector _labelMapX;
    WeightVector _labelMapY;
    MateVector _mateMapX;
    MateVector _mateMapY;
    int _nX;
    int _nY;
    int _matchingSize;
    Value _matchingWeight;

    static const Value _maxValue;

    void buildMatchingMap()
    {
      _matchingWeight = 0;
      _matchingSize = 0;

      for (int x = 0; x < _nX; x++)
      {
        assert(_mateMapX[x] != -1);
        int y = _mateMapX[x];

        const Edge& e = _adjacencyMatrix[x][y].getEdge();
        if (e != INVALID)
        {
          // only edges that where present
          // in the original graph count in the matching
          _matchingMap[_graph.u(e)] = _matchingMap[_graph.v(e)] = e;
          _matchingSize++;
          _matchingWeight += _weight[e];
        }
      }
    }

    void updateSlacks(WeightVector& slack, int x)
    {
      Value lx = _labelMapX[x];
      for (int y = 0; y < _nY; y++)
      {
        // slack[y] = min_{x \in S} [l(x) + l(y) - w(x, y)]
        Value val = lx + _labelMapY[y] - _adjacencyMatrix[x][y].getWeight();
        if (slack[y] > val)
          slack[y] = val;
      }
    }

    void updateLabels(const BoolVector& setS,
                      const BoolVector& setT, WeightVector& slack)
    {
      // recall that slack[y] = min_{x \in S} [l(x) + l(y) - w(x,y)]

      // delta = min_{y \not \in T} (slack[y])
      Value delta = _maxValue;
      for (int y = 0; y < _nY; y++)
      {
        if (!setT[y] && slack[y] < delta)
          delta = slack[y];
      }

      // update labels in X
      for (int x = 0; x < _nX; x++)
      {
        if (setS[x])
          _labelMapX[x] -= delta;
      }

      // update labels in Y
      for (int y = 0; y < _nY; y++)
      {
        if (setT[y])
          _labelMapY[y] += delta;
        else
        {
          // update slacks
          // remember that l(x) + l(y) hasn't changed for x \in S and y \in T
          // the only thing that has changed is"
          //   l(x) + l(y) for x \in S and y \not \in T
          slack[y] -= delta;
        }
      }
    }

  public:
    /// \brief Constructor
    ///
    /// Constructor.
    ///
    /// \param graph is the input graph
    /// \param weight are the edge weights
    MaxWeightedDenseBipartiteMatching(const BpGraph& graph,
                                      const WeightMap& weight)
      : _graph(graph)
      , _weight(weight)
      , _idMap(graph, -1)
      , _matchingMap(graph, INVALID)
      , _adjacencyMatrix()
      , _labelMapX()
      , _labelMapY()
      , _mateMapX()
      , _mateMapY()
      , _nX(0)
      , _nY(0)
      , _matchingSize(0)
      , _matchingWeight(0)
    {

    }

    /// \brief Initialize the algorithm
    ///
    /// This function initializes the algorithm.
    void init()
    {
      // construct _idMap
      int id_x = 0;
      for (RedNodeIt x(_graph); x != INVALID; ++x, ++id_x)
      {
        _idMap[x] = id_x;
      }
      _nX = id_x;

      int id_y = 0;
      for (BlueNodeIt y(_graph); y != INVALID; ++y, ++id_y)
      {
        _idMap[y] = id_y;
      }
      _nY = id_y;

      assert(_nX <= _nY);

      // init matching is empty
      _mateMapX = MateVector(_nX, -1);
      _mateMapY = MateVector(_nY, -1);

      // labels of nodes in X are initialized to 0,
      // these will be updated during initAdjacencyMatrix()
      _labelMapX = WeightVector(_nX, 0);

      // labels of nodes in Y are initialized to 0,
      // these won't be updated during initAdjacencyMatrix()
      _labelMapY = WeightVector(_nY, 0);

      // adjacency matrix has dimensions |X| * |Y|,
      // every entry in this matrix is initialized to (0, INVALID)
      _adjacencyMatrix = AdjacencyMatrixType(_nX,
        std::vector<BpEdgeT>(_nY, BpEdgeT()));

      _matchingWeight = 0;
      _matchingSize = 0;

      for (RedNodeIt x(_graph); x != INVALID; ++x)
      {
        id_x = _idMap[x];
        for (IncEdgeIt e(_graph, x); e != INVALID; ++e)
        {
          Node y = _graph.blueNode(e);
          id_y = _idMap[y];

          Value w = _weight[e];

          BpEdgeT& item = _adjacencyMatrix[id_x][id_y];
          item.setEdge(e);
          item.setWeight(w);

          // label of a node x in X is initialized to maximum weight
          // of edges incident to x
          if (w > _labelMapX[id_x])
            _labelMapX[id_x] = w;
        }
      }
    }

    /// \brief Run the algorithm.
    ///
    /// This method runs the \c %MaxWeightedDenseBipartiteMatching algorithm.
    ///
    /// \note mwdbm.run() is just a shortcut of the following code.
    /// \code
    ///   mwdbm.init()
    ///   mwdbm.start();
    /// \endcode
    void run()
    {
      init();
      start();
    }

    /// \brief Start the algorithm
    ///
    /// This function starts the algorithm.
    ///
    /// \pre \ref init() must have been called before using this function.
    void start()
    {
      // maps y in Y to x in X by which it was discovered
      MateVector discoveredY(_nY, -1);

      // pick a root
      for (int r = 0; r < _nX; )
      {
        assert(_mateMapX[r] == -1);

        // clear slack map, i.e. set all slacks to +INF
        WeightVector slack(_nY, _maxValue);

        // initially T = {}
        BoolVector setT(_nY, false);

        // initially S = {r}
        BoolVector setS(_nX, false);
        setS[r] = true;

        std::queue<int> queue;
        queue.push(r);

        updateSlacks(slack, r);

        bool augmented = false;
        while (!queue.empty() && !augmented)
        {
          int x = queue.front();
          queue.pop();

          for (int y = 0; y < _nY; y++)
          {
            if (!setT[y] &&
              _labelMapX[x] + _labelMapY[y] ==
              _adjacencyMatrix[x][y].getWeight())
            {
              // y was (first) discovered by x
              discoveredY[y] = x;

              if (_mateMapY[y] != -1) // y is matched, extend alternating tree
              {
                int z = _mateMapY[y];

                // add z to queue if not in S
                if (!setS[z])
                {
                  setS[z] = true;
                  queue.push(z);
                  updateSlacks(slack, z);
                }

                setT[y] = true;
              }
              else // y is free, we have an augmenting path between r and y
              {
                int cx, ty, cy = y;
                do {
                  cx = discoveredY[cy];
                  ty = _mateMapX[cx];

                  _mateMapX[cx] = cy;
                  _mateMapY[cy] = cx;

                  cy = ty;
                } while (cx != r);

                // we found an augmenting path,
                // start a new iteration of the first for loop
                augmented = true;
                break; // break for y
              }
            }
          } // y \not in T such that (r,y) in E_l
        } // queue

        if (!augmented)
          updateLabels(setS, setT, slack);
        else
          r++;
      }

      buildMatchingMap();
    }

    /// \brief Return the dual value of the given node
    ///
    /// This function returns the potential of the given node
    ///
    /// \pre init() must have been called before using this function
    const Value pot(const Node& n) const
    {
      if (_graph.red(n))
        return _labelMapX[_idMap[n]];
      else
        return _labelMapY[_idMap[n]];
    }

    /// \brief Return the weight of the matching.
    ///
    /// This function returns the weight of the found matching.
    ///
    /// \pre init() must have been called before using this function.
    Value matchingWeight() const
    {
      return _matchingWeight;
    }

    /// \brief Return the number of edges in the matching.
    ///
    /// This function returns the number of edges in the matching.
    int matchingSize() const
    {
      return _matchingSize;
    }

    /// \brief Return \c true if the given edge is in the matching.
    ///
    /// This function returns \c true if the given edge is in the found
    /// matching.
    ///
    /// \pre init() must have been been called before using this function.
    bool matching(const Edge& e) const
    {
      return _matchingMap[_graph.u(e)] != INVALID;
    }

    /// \brief Return the matching edge incident to the given node.
    ///
    /// This function returns the matching edge incident to the
    /// given node in the found matching or \c INVALID if the node is
    /// not covered by the matching.
    ///
    /// \pre init() must have been been called before using this function.
    Edge matching(const Node& n) const
    {
      return _matchingMap[n];
    }

    /// \brief Return the mate of the given node.
    ///
    /// This function returns the mate of the given node in the found
    /// matching or \c INVALID if the node is not covered by the matching.
    ///
    /// \pre init() must have been been called before using this function.
    Node mate(const Node& n) const
    {
      return _graph.oppositeNode(n, _matchingMap[n]);
    }

    /// \brief Return a const reference to the matching map.
    ///
    /// This function returns a const reference to a node map that stores
    /// the matching edge incident to each node.
    ///
    /// \pre init() must have been called before using this function.
    const MatchingMap& matchingMap() const
    {
      return _matchingMap;
    }

    /// \brief Checks whether the solution is optimal
    ///
    /// Checks using the dual solution whether the primal solution is optimal.
    ///
    /// \return \c true if the solution is optimal.
    bool checkOptimality() const
    {
      for (RedNodeIt x(_graph); x != INVALID; ++x)
      {
        if (_labelMapX[_idMap[x]] < 0)
          return false; // feasibility of dual solution

        const Edge e = _matchingMap[x];

        if (_mateMapX[_idMap[x]] == -1)
          return false; // all nodes in X must be matched in the complete graph
        else if (e != INVALID)
        {
          const Node y = _graph.blueNode(e);

          if (_matchingMap[y] != e)
            return false; // if x is matched via e then so must y
          if (_labelMapX[_idMap[x]] + _labelMapY[_idMap[y]] !=
              _adjacencyMatrix[_idMap[x]][_idMap[y]].getWeight())
            return false; // primal complementary slackness
        }
      }

      for (BlueNodeIt y(_graph); y != INVALID; ++y)
      {
        if (_labelMapY[_idMap[y]] < 0)
          return false; // feasibility of dual solution

        if (_matchingMap[y] == INVALID && _labelMapY[_idMap[y]] != 0)
          return false; // dual complementary slackness
      }

      for (EdgeIt e(_graph); e != INVALID; ++e)
      {
        // feasibility of dual solution
        if (_labelMapX[_idMap[_graph.redNode(e)]] +
            _labelMapY[_idMap[_graph.blueNode(e)]] < _weight[e])
            return false;
      }

      return true;
    }
  };

  template<typename BGR, typename WM>
  const typename WM::Value MaxWeightedDenseBipartiteMatching<BGR, WM>::
    _maxValue = std::numeric_limits<typename WM::Value>::max();
}

#endif //BP_MATCHING_H
