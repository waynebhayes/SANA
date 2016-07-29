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
  double key;
  std::pair<ushort,ushort> value;

  static constexpr int MAX_LEVEL = 16;
	
  SkipNode();
  SkipNode(int h, double k, std::pair<ushort,ushort> v);
  //SkipNode(int h, double k, std::pair<ushort,ushort> & v);
  //SkipNode(int h, double k, ushort n1, ushort n2, SkipNode * vec[]);
  ~SkipNode();
  SkipNode * forward[MAX_LEVEL];
  void debug(int limit);
};

class SkipList{
public:
  /*
  SkipList();
  SkipList(double delta);
  SkipList(double delta, bool setMaxHeap);
  */
  SkipList(double delta, bool setMaxHeap,
	   std::unordered_set<ushort> & lex, std::unordered_set<ushort> & rex);
  ~SkipList();
	
  void removeNode(SkipNode * n);
  void insert(double, std::pair<ushort,ushort> entry);
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
  double delta;
	
  SkipNode head;
  static constexpr double SKIPLIST_P = 0.25;
  static constexpr double EPSILON = 0.0000001;
  static constexpr uint CLEAN_THRESH = 100;
  std::mt19937 rng;
  std::chrono::milliseconds insert_time;
  uint miss_counter;
  std::unordered_set<ushort> & left_exclude;
  std::unordered_set<ushort> & right_exclude;
  std::vector<std::uniform_int_distribution<int>> uni;
};

#endif
