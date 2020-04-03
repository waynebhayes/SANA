This is a tutorial for adding a measure

Before getting started with the code for the new measure make sure that you are familiar with the structure of this repository. In particular, learn how the code works inside SANA.hpp/.cpp, Graph.hpp/.cpp, Alignment.cpp/.hpp. The Graph.cpp and Alignment.cpp files contains the tools you might need to implement the new measure. SANA.cpp contains code incremental changes from vaious measures. The incremental change code is something you will add to when implement the measure

After getting familar with the tools available in Alignment and Graph cpp files, plan out your code in a way that leverages those tools as much as possible to avoid future confusion. If you decide that you will need some extra functionality to be able to implement the new measure, consider adding that to the Graph or Alignment files (if this new functionality can be used in other situations and not just the new measure)

If the new measure is a local measure, the /src/measures/localMeasures will contain code for other local measures, read through a few of them to understand the implementation structure. For local measures, the incremental updates are handled in SANA.cpp are combined for all local measures and they are handled together in the code. Look at the variable localScoreSum which stores the combined sum of all local measures (TIP search "needLocal" in SANA.cpp and read the code around each search result to understand how the incremental update code for all local measure works)
If the new measure is a normal measure, the /src/measures/will contain code for other normal measures, read through a few of them to understand the implementation structure. Implementation of these is a little more tricky than local measures. These measures will require their own seperate code in SANA.cpp.

STEPs (for normal measures, can be modified to work for local measures through intuition):

1 - In the folder /src/measures, Create a .hpp and .cpp file for your new measure. You will need to define a class for the new measure, this class inherits from the class Measure. The constructor takes two arguments Graph* G1 and Graph*G2. These are the graphs that SANA will align. If the new measure class has non-static private variables the constructor would be the place to assign those variables a value.
2 - In the new measure class, have a function called eval which takes argument - const Alignment& A. This function evaluates an Alignment from scratch. This function is called when the Alignment is starting and periodically to check if incremental updates haver been correct. This function will return a double. This return is the evaluation of this Alignment's quality using this measure.
3 - After setting up the basic framework for the new measure .hpp and .cpp files. Add the measure into defaultArguments.cpp inside /src/arguments. 
4 - Add information about the measure in supportedArguments.cpp. The format for the way this information needs to be added should be apparent inside the file.
5 - Add the code for defining as instance of the measure inside the file measureSelector.cpp (and measureSelector.hpp if need). The sample code for this is available on lines 197-289. This should be relatively simple unless this new measure needs some special information to be instanciated.
6 - Add the new measure .cpp file as a compile target in the makefile (/SANA/makefile). SEE line 52 and onwards where it says MEASURES_SRCS =.
7 - Finish coding .cpp file. Look inside the base Measure class to see which variables you have access to.
8 - Next thing adding code to make this measure usuable by SANA. To do this Identify which mode(s) this measure will be used in. Current modes offered are TabuSearch, HillClimbing, Pareto, normal. The normal, Pareto mode are inside SANA.cpp. For this tutorial we will be focusing on normal mode.
9 - Add the variables you will need to SANA.hpp. If the new varabiles can be defined without an Alignment then define them inside the constructor in SANA.cpp(SEE needWec, needAligEdges variables). If these new variables need an Alignement to be defined you should define them inside the "nitDataStructures" function(SEE alignEdges, edSum, erSum variables).
10 - In normal mode you will keep track of the currect score assigned to the alignement using your measure with a *newMeasure*Sum varible(SEE s3Sum, edSum, erSum, wecSum etc). These variables will store the inital value that your new measure's eval function gives out.
11 - For the incremental code itself, you will need to define 2 new functions (and 2 more for pareto mode). These functions will be called *newMeasure*IncChangeOP and *newMeasure*IncSwapOP. As the name implies the change function tracks the incremental change when a source from G1 is changed to a new target in G2. The swap function tracks the incremental change when two sources from G1 are swapped to other's target in G2. For Example Code, see edgeDifferenceIncChangeOp, edgeDifferenceIncSwapOp.
12 - After this add code to the performChange function in lines 1360 - 1372. SEE(newEdSum variable on line 1361 for example).
13 - Add code to performSwap function in lines 1494 - 1505. SEE(newEdSum variable on line 1504 for example).
14 - Add code to the scoreComparision function
15 - Compile using the Makefile and Debug.


EXTRAS:

1 - FOR pareto mode you might have to add code to save the variables you need. For hints look at the struct AlignmentInfo in SANA.hpp and copyAlignmentFromStoragefunction in SANA.cpp.

2 - The code compilation in SANA.cpp goes in the order of SANA constructor, getStartingAlignment, initDataStructures, run. The run functions runs the alignent and calles on various different functions to perform incremental changes.

3 - If you needed to use any pointers in any classes, make sure to delete thos pointers in the class destructors to avoid memory leaks.