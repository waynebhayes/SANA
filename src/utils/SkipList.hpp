#ifndef SKIPLIST_HPP
#define SKIPLIST_HPP

#include <limits.h>
#include <iostream>
#include <fstream>
#include <random>
#include <exception>
#include <unordered_set>

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
  std::pair<ushort,ushort> value;

  static constexpr int MAX_LEVEL = 16;
	
  SkipNode();
  SkipNode(int h, float k, std::pair<ushort,ushort> v);
  ~SkipNode();
  SkipNode * forward[MAX_LEVEL];
  void debug(int limit);
};

class SkipList{
public:
  SkipList(float delta, bool setMaxHeap,
	   std::unordered_set<ushort> & lex, std::unordered_set<ushort> & rex);
  ~SkipList();
	
  void removeNode(SkipNode * n);
  void insert(float similarity, std::pair<ushort,ushort> entry);
  bool empty();
  std::pair<ushort,ushort> pop_uniform();
  std::pair<ushort,ushort> pop_distr();
  bool isMaxHeap();

  void test();
  void debug();
  void perf();
  void showCounter();

  void serialize(std::string fname);
  bool deserialize(std::string fname);

protected:
  int random_height(void);
  void serialize_helper(SkipNode * curr, std::ofstream & out);
  uint random_int(uint n); 

private:
  void cleanup();

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
  uint miss_counter;
  std::unordered_set<ushort> & left_exclude;
  std::unordered_set<ushort> & right_exclude;
  std::vector<std::uniform_int_distribution<int>> uni;
};

#endif
