#Read Me
Requires graphviz to be installed.
Compile DrawGraphette.cpp to graphette2dot.
Must be compiled with C++11 for stoull for parsing decimal input above k=8.
EX:
```
g++ -std=c++11 DrawGraphette.cpp -o graphette2dot
```
TestDrawGraphette.cpp is an example of how to programatically permuate canonicals or bitstrings with graphette2dot.
You must convert the dot files with graphviz afterwards.