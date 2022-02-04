# Text Indexing Programming Project

This directory contains all files needed for the implementation project.

## Overview

- The input is read, queries are started and the output is generated in main.cpp.
- I use a suffix tree-based approach. The suffix tree is generated using Ukkonen's algorithm in `UkkonenSuffixTree/SuffixTree.h`. 
    * I do not explain the basics of the algorithm in my documentation.
    * The implementation follows the structure that is described in the referenced sources.
- There is a naive O(n^2) suffix tree-generation that I used earlier in `NaiveSuffixTree/SuffixTree.h`; it can be ignored.
- The Queries are implemented in `Queries/TopKQuery.h` and `Queries/RepeatQuery.h`, respectively.
    * My approaches for the queries are explained in detail in the code.
- I used Profilers (`Helpers/RepeatProfiler.h`, `Helpers/TopKProfiler.h`) during the optimization of the queries. The current variant uses the dummy variants to avoid overheads.

## Requirements

My implementation is in C++20 and uses CMake. It runs on my Ubuntu 20.04 WSL2 with g++ 10.3.0.

## Building and Execution

To build, run the following commands from the main folder `Framework`:
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Run the program using 
```
./build/Framework [topk|repeat] path_to_input_file
```

For instance:
```
./build/Framework topk ./TestFiles/topK-trivial.txt
./build/Framework repeat ./TestFiles/repeat-trivial.txt
```

## About the Running Times...

I allocate everything to construction time that I consider reasonable there.
Besides the suffix tree construction time, this also includes query initialization time for work that needs to be done only once for all queries (in the case of topK-queries) or could be done during suffix tree construction if it was only used for repeat queries.