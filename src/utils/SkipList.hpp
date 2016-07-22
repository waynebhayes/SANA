#ifndef SKIPLIST_HPP
#define SKIPLIST_HPP

//#include <stdio.h>
#include <stdlib.h>  //for rand
//#include <math.h>
#include <limits.h>
#include <iostream>
#include <random>
#include <exception>

class SkipNode{
public:
	int height;
	double key;
	std::pair<ushort,ushort> value;
	
	static constexpr int MAX_LEVEL = 16;
	
	SkipNode();
	SkipNode(int h, double k, std::pair<ushort,ushort> & v);
	~SkipNode();
	SkipNode * forward[MAX_LEVEL];
	void debug(int limit);
	
};

class SkipList{
public:
	SkipList();
	SkipList(double delta);
	SkipList(double delta, bool setMaxHeap);
	~SkipList();
	
	void removeNode(SkipNode * n);
	void insert(double, std::pair<ushort,ushort> entry);
	bool empty();
	std::pair<ushort,ushort> pop_uniform();
	std::pair<ushort,ushort> pop_distr();
	bool isMaxHeap();

	static void test();
	void debug();
	
protected:
	static int random_height(void);
	
private:
	bool _isMaxHeap;
	long length;
	int level;
	double delta;
	
	SkipNode head;
	static constexpr double SKIPLIST_P = 0.25;
	static constexpr double EPSILON = 0.0000001;
	std::mt19937 rng;
};

#endif
