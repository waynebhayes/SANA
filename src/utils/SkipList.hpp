#ifndef SKIPLIST_HPP
#define SKIPLIST_HPP

#include <limits.h>
#include <iostream>
#include <fstream>
#include <random>
#include <exception>
#include <unordered_set>

#include "randomSeed.hpp"

#include <chrono>

class QueueEmptyException : public std::exception{
public:
  virtual const char* what() const throw()
  {
    return "queue is empty";
  }
};

class SkipNode{
public:
  int height;
  float key;
  std::pair<uint,uint> value;

  static constexpr int MAX_LEVEL = 16;
    
  SkipNode();
  SkipNode(int h, float k, std::pair<uint,uint> v);
  ~SkipNode();
  SkipNode * forward[MAX_LEVEL];
  void debug(int limit);
};

class SkipList{
public:
  SkipList(float delta, bool setMaxHeap,
       std::unordered_set<uint> & lex, std::unordered_set<uint> & rex);
  ~SkipList();
    

  void insert(float similarity, std::pair<uint,uint> entry);
  bool empty();
  std::pair<uint,uint> pop_reservoir();
  std::pair<uint,uint> pop_distr();
  bool isMaxHeap();

  void debug();
  void perf();

  void serialize(std::string fname);
  bool deserialize(std::string fname);

protected:
  int random_height(void);
  uint random_int(uint n); 

private:
  void removeNode(SkipNode * n);
  void partial_cleanup(float tail);
  void cleanup();
  void test();

  bool _isMaxHeap;
  long length;
  int level;
  float delta;
    
  SkipNode head;
  static constexpr float SKIPLIST_P = 0.25;
  static constexpr float EPSILON = 0.0000001;
  static constexpr uint CLEANUP_THRESH = 1000;
  std::mt19937 rng;
  std::chrono::milliseconds insert_time;
  std::chrono::milliseconds cleanup_time;
  std::chrono::milliseconds pop_time;
  uint cumulative_miss;
  uint miss_counter;
  std::unordered_set<uint> & left_exclude;
  std::unordered_set<uint> & right_exclude;
  std::vector<std::uniform_int_distribution<int>> uni;
};

#endif
