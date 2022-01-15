#include <iostream>
#include <fstream>
#include <chrono>

#include "NaiveSuffixTree/SuffixTree.h"
#include "Query/TopKQuery.h"
#include "Query/RepeatQuery.h"
#include "Helpers/Timer.h"

struct TopKQuery {
    size_t l;
    size_t k;
};

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "Wrong number of arguments, expecting 2 arguments." << std::endl;
        return 1;
    }

    const bool Debug = false;
    const bool PreprocessingDebug = false;

    std::string queryChoice(argv[1]);
    if (queryChoice.compare("topk") == 0) {
        if constexpr (Debug) std::cout << "Requested topk query." << std::endl;

        std::string inputFileName(argv[2]);
        std::ifstream inputFile(inputFileName);

        size_t numberOfQueries;
        inputFile >> numberOfQueries;
        std::vector<TopKQuery> queries;
        queries.reserve(numberOfQueries);
        //read all the queries
        for (size_t i = 0; i < numberOfQueries; i++) {
            size_t k, l;
            inputFile >> l;
            inputFile >> k;
            queries.emplace_back(l, k);
        }
        if constexpr (Debug) std::cout << "Found " << numberOfQueries << " queries." << std::endl;

        //read remaining part of the input
        /*
        std::stringstream inputBuffer;
        inputBuffer << inputFile.rdbuf();
        std::string inputText = inputBuffer.str();
        */
        std::string inputText;
        inputFile >> inputText;
        if constexpr (Debug) std::cout << "Read input file: '" << inputText << "'" << std::endl;

        Helpers::Timer preprocessingTimer;
        NaiveSuffixTree::SuffixTree<char, PreprocessingDebug> stree(inputText.c_str(), inputText.length());
        size_t preprocessingTime = preprocessingTimer.getMilliseconds();

        //generate query
        Helpers::Timer queryInitTimer;
        Query::TopKQuery<char, Debug> query(&stree);
        size_t queryInitTime = queryInitTimer.getMilliseconds();

        size_t totalQueryTime = 0;
        Helpers::Timer queryTimer;
        //run queries
        for (size_t i = 0; i < numberOfQueries; i++) {
            queryTimer.restart();
            size_t startIndex = query.runQuery(queries[i].l, queries[i].k);
            totalQueryTime += queryTimer.getMilliseconds();
            std::cout << "Query l=" << queries[i].l << ", k=" << queries[i].k << ": " << stree.substring(startIndex, queries[i].l) << " (" << startIndex << ")" << std::endl;
        }

        std::cout << "Preprocessing time: " << preprocessingTime << std::endl;
        std::cout << "Query init. time:   " << queryInitTime << std::endl;
        std::cout << "Total query time:   " << totalQueryTime << std::endl;
        std::cout << "Avg. query time:    " << totalQueryTime / (double) numberOfQueries << std::endl;
    } else if (queryChoice.compare("repeat") == 0) {
        std::cout << "Requested repeat query." << std::endl;

        std::string inputFileName(argv[2]);
        std::ifstream inputFile(inputFileName);

        //read remaining part of the input
        /*
        std::stringstream inputBuffer;
        inputBuffer << inputFile.rdbuf();
        std::string inputText = inputBuffer.str();
        */
        std::string inputText;
        inputFile >> inputText;
        if constexpr (Debug) std::cout << "Read input file: '" << inputText << "'" << std::endl;

        Helpers::Timer preprocessingTimer;
        NaiveSuffixTree::SuffixTree<char, PreprocessingDebug> stree(inputText.c_str(), inputText.length());
        size_t preprocessingTime = preprocessingTimer.getMilliseconds();

        //generate query
        Helpers::Timer queryInitTimer;
        Query::RepeatQuery<char, Debug> query(&stree);
        size_t queryInitTime = queryInitTimer.getMilliseconds();

        size_t startPosition, length;
        Helpers::Timer queryTimer;
        std::tie(startPosition, length) = query.runQuery();
        size_t queryTime = queryTimer.getMilliseconds();
        std::cout << "Query result: " << stree.substring(startPosition, length) << stree.substring(startPosition, length) << " (" << startPosition << ", " << length << ")" << std::endl << std::endl;

        std::cout << "Preprocessing time: " << preprocessingTime << std::endl;
        std::cout << "Query init. time:   " << queryInitTime << std::endl;
        std::cout << "Total query time:   " << queryTime << std::endl;
    } else {
        std::cout << "Unknown query choice." << std::endl;
        return 1;
    }

    return 0;
}
