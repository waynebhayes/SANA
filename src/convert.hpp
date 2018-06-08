#ifndef CONVERT_HPP
#define CONVERT_HPP
#include<cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <regex>
#include<cstdio>
#include <bits/stdc++.h>
#include <sys/resource.h>
using namespace std;
int find_type(string input);
void convert_leda( string filename);
void convert_lgf( string filename);
void convert_grapgml(string inp);
void convert_gml(string inp);
void convert_csv( string filename);
std::map<int,std::string> toString(string filename);

string convert(string filename);
#endif