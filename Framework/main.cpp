#include <iostream>
#include <fstream>

#include "NaiveSuffixTree/SuffixTree.h"
#include "Query/TopKQuery.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "Wrong number of arguments, expecting 2 arguments." << std::endl;
        return 1;
    }

    const bool Debug = false;

    std::string queryChoice(argv[1]);
    std::string inputFileName(argv[2]);
    std::ifstream inputFile(inputFileName);
    std::stringstream inputBuffer;
    inputBuffer << inputFile.rdbuf();
    std::string inputText = inputBuffer.str();
    std::cout << "Read input file: " << inputText << std::endl;

    if (queryChoice.compare("topk") == 0) {
        std::cout << "Requested topk query." << std::endl;
        NaiveSuffixTree::SuffixTree<char, Debug> stree(inputText.c_str(), inputText.length());

        Query::TopKQuery<char, Debug> query(&stree);
        query.runQuery(2, 3);
    } else if (queryChoice.compare("repeat") == 0) {
        std::cout << "Requested repeat query." << std::endl;
        NaiveSuffixTree::SuffixTree<char, Debug> stree(inputText.c_str(), inputText.length());
    } else {
        std::cout << "Unknown query choice." << std::endl;
        return 1;
    }

    return 0;
}
