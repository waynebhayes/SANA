#include <iostream>
#include <chrono>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <string>
#include "graphette.hpp"


int main(int arg, char* argv[])
{
    //Arguments used for each sub pass 
    int num_nodes = atoi(argv[1]);
    int variation = atoi(argv[2]);
    int block_size = atoi(argv[3]);
    int start = atoi(argv[4]);
    int end = atoi(argv[5]);
    
    generate_initial_canonical(num_nodes, variation, block_size, start, end);
    
    return 0;
}




