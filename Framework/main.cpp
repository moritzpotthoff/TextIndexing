#include <iostream>
#include <fstream>

//#include "NaiveSuffixTree/SuffixTree.h"
#include "Query/TopKQuery.h"
#include "Query/RepeatQuery.h"
#include "Helpers/Timer.h"
#include "Helpers/TopKProfiler.h"
#include "Helpers/RepeatProfiler.h"

#include "UkkonenSuffixTree/SuffixTree.h"
#include "UkkonenSuffixTree/Node.h"

/**
 * One topK Query for length l and the k-th candidate.
 */
struct TopKQuery {
    size_t l;
    size_t k;
};

//Interactive flag. If true, generates a little more output than just the result line.
static const bool Interactive = false;
//Debug flag. Generates extensive debug info.
static const bool Debug = Interactive && false;
using CharType = char;
static const CharType Sentinel = '\0';

inline static void readRemainingFileContents(std::ifstream& inputFile, std::string& inputText) {
    std::stringstream inputBuffer;
    inputBuffer << inputFile.rdbuf();
    inputText = inputBuffer.str();
    inputText.push_back(Sentinel);//add sentinel for preprocessing
}

inline static void handleTopKQuery(char *argv[]) {
    if constexpr (Interactive) std::cout << "Requested topk query." << std::endl;

    std::string inputFileName(argv[2]);
    std::ifstream inputFile(inputFileName);

    //Read the queries from the file.
    size_t numberOfQueries;
    inputFile >> numberOfQueries;
    std::vector<TopKQuery> queries;
    queries.reserve(numberOfQueries);
    for (size_t i = 0; i < numberOfQueries; i++) {
        size_t k, l;
        inputFile >> l;
        inputFile >> k;
        queries.emplace_back(l, k);
    }
    if constexpr (Interactive) std::cout << "Found " << numberOfQueries << " queries." << std::endl;

    //Read the actual text.
    std::string inputText;
    readRemainingFileContents(inputFile, inputText);

    //Used for time for the output.
    Helpers::Timer preprocessingTimer;
    //Generate the suffix tree for the input.
    //Use +/-2 in start and length to cut off the line break between the last query part and the actual text.
    SuffixTree::SuffixTree<CharType, Debug> stree(inputText.c_str() + 2, inputText.length() - 2);
    size_t preprocessingTime = preprocessingTimer.getMilliseconds();
    if constexpr (Debug) std::cout << "Generated suffix tree for input: '" << stree.text << "'." << std::endl;

    //The time needed (once) for additional query preprocessing will be added to the suffix tree generation time for the total preprocessing time.
    Helpers::Timer queryInitTimer;
    //Generate the query instance.
    Query::TopKQuery<CharType, Sentinel, Query::TopKNoProfiler, Debug> query(&stree);
    size_t queryInitTime = queryInitTimer.getMilliseconds();

    if constexpr (Debug) stree.printSimple();

    size_t totalQueryTime = 0;
    Helpers::Timer queryTimer;
    //Run the queries.
    std::stringstream queryResults;
    for (size_t i = 0; i < numberOfQueries; i++) {
        queryTimer.restart();
        size_t startIndex = query.runQuery(queries[i].l, queries[i].k);
        totalQueryTime += queryTimer.getMilliseconds();
        queryResults << stree.substring(startIndex, queries[i].l);
        if (i < numberOfQueries - 1) queryResults << ";";
        if constexpr (Interactive) std::cout << "Query l=" << queries[i].l << ", k=" << queries[i].k << ": " << stree.substring(startIndex, queries[i].l) << " (" << startIndex << ")" << std::endl;
    }

    if constexpr (Interactive) {
        std::cout << std::endl;
        std::cout << "Preprocessing time: " << preprocessingTime << std::endl;
        std::cout << "Query init. time:   " << queryInitTime << std::endl;
        std::cout << "Total query time:   " << totalQueryTime << std::endl;
        std::cout << "Avg. query time:    " << totalQueryTime / (double) numberOfQueries << std::endl;
        std::cout << std::endl;
        query.profiler.print();
        std::cout << std::endl;
    }

    std::cout   << "RESULT algo=topk name=moritz-potthoff"
                << " construction time=" << (preprocessingTime + queryInitTime)//count the initialization of the query (that is independent of actual queries) as preprocessing time
                << " query time=" << totalQueryTime
                << " solutions=" << queryResults.str()
                << " file=" << inputFileName << std::endl;
}

inline static void handleRepeatQuery(char *argv[]) {
    if constexpr (Interactive) std::cout << "Requested repeat query." << std::endl;

    std::string inputFileName(argv[2]);
    std::ifstream inputFile(inputFileName);

    //Read the input text.
    std::string inputText;
    readRemainingFileContents(inputFile, inputText);
    if constexpr (Debug) std::cout << "Read input file: '" << inputText << "'" << std::endl;

    //Measure the preprocessing time.
    Helpers::Timer preprocessingTimer;
    //Generate the suffix tree.

    SuffixTree::SuffixTree<CharType, Debug> stree(inputText.c_str(), inputText.length());
    size_t preprocessingTime = preprocessingTimer.getMilliseconds();
    if constexpr (Debug) std::cout << "Generated suffix tree for input: '" << stree.text << "'" << std::endl;

    if constexpr (Interactive) std::cout << "Preprocessing done." << std::endl;
    //Again, query initialization time will be measured as preprocessing time.
    Helpers::Timer queryInitTimer;
    //Generate the query instance.
    Query::RepeatQuery<CharType, Sentinel, Query::RepeatNoProfiler, Debug> query(&stree);
    size_t queryInitTime = queryInitTimer.getMilliseconds();

    if constexpr (Debug) stree.printSimple();

    size_t startPosition, length;
    Helpers::Timer queryTimer;
    //Compute the query.
    std::tie(startPosition, length) = query.runQuery();
    size_t queryTime = queryTimer.getMilliseconds();
    if constexpr (Interactive) {
        std::cout << "Query result: " << stree.substring(startPosition, length) << " (" << startPosition << ", " << length << ")" << std::endl << std::endl;
        std::cout << std::endl;
        std::cout << "Preprocessing time: " << preprocessingTime << std::endl;
        std::cout << "Query init. time:   " << queryInitTime << std::endl;
        std::cout << "Total query time:   " << queryTime << std::endl;
        std::cout << std::endl;
        query.profiler.print();
        std::cout << std::endl;
    }
    std::cout << "RESULT algo=repeat name=moritz-potthoff"
              << " construction time=" << (preprocessingTime + queryInitTime)//count query initialization as preprocessing: It could be done during the suffix tree generation, if that was only used for repeat queries.
              << " query time=" << queryTime
              << " solution=" << stree.substring(startPosition, length)
              << " file=" << inputFileName << std::endl;
}

inline static std::string getPrefix(std::string input, int length) noexcept {
    if (length >= input.length()) std::cout << "ERROR: insufficient input." << std::endl;
    std::string result(input);
    result.resize(length);
    return result;
}

static std::vector<int> inputTextLengths = { 100000, 500000, 1000000 };
static std::vector<int> inputTextLengths2 = { 10000000, 15000000 };
static std::vector<int> inputTextLengths3 = { 20000000 };
static std::vector<int> inputTextLengths4 = { 5000000 };
static std::vector<int> queryInputTextLengths = { 1000, 1000000, 2500000, 5000000, 7500000, 10000000 };
static std::vector<int> queryInputTextLengthsTopK = { 10000000 };
static std::vector<int> topKQueryLengths = { 1, 5, 10, 50, 100, 500, 1000, 5000, 10000, 50000, 100000, 500000, 1000000 };
static std::vector<int> topKQueryLengthsSmall = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
static std::vector<int> topKQueryLengths2 = { 10 };

inline static void preprocessingExperiment(char *argv[]) {
    std::cout << "Requested preprocessing experiment." << std::endl;

    std::string inputFileName(argv[2]);
    std::ifstream inputFile(inputFileName);

    //Read the input text.
    std::string inputText;
    readRemainingFileContents(inputFile, inputText);
    std::cout << "Original input has length " << inputText.length() << std::endl;

    for (int inputLength : inputTextLengths4) {
        std::string prefix = getPrefix(inputText, inputLength);

        Helpers::Timer preprocessingTimer;
        SuffixTree::SuffixTree<CharType, Debug> stree(prefix.c_str(), prefix.length());
        size_t preprocessingTime = preprocessingTimer.getMilliseconds();

        std::cout << "RESULT algo=preprocessingExperiment"
                  << " constructionTime=" << preprocessingTime
                  << " inputType=" << argv[3]
                  << " inputLength=" << prefix.length()
                  << " file=" << inputFileName << std::endl;
    }
}

inline static void topKQueryExperiment(char *argv[]) {
    std::cout << "Requested topKQuery experiment." << std::endl;

    std::string inputFileName(argv[2]);
    std::ifstream inputFile(inputFileName);

    //Read the input text.
    std::string inputText;
    readRemainingFileContents(inputFile, inputText);
    std::cout << "Original input has length " << inputText.length() << std::endl;

    for (int inputLength : queryInputTextLengthsTopK) {
        std::string prefix = getPrefix(inputText, inputLength);
        SuffixTree::SuffixTree<CharType, Debug> stree(prefix.c_str(), prefix.length());
        Query::TopKQuery<CharType, Sentinel, Query::TopKNoProfiler, Debug> query(&stree);

        //Run the queries.
        Helpers::Timer queryTimer;
        for (int queryLength : topKQueryLengthsSmall) {
            if (queryLength >= inputLength) break;
            queryTimer.restart();
            size_t startIndex = query.runQuery(queryLength, 1);
            int queryTime = queryTimer.getMilliseconds();
            std::cout << "RESULT algo=topKQueryExperiment"
                      << " queryTime=" << queryTime
                      << " inputType=" << argv[3]
                      << " inputLength=" << prefix.length()
                      << " queryLength=" << queryLength
                      << " file=" << inputFileName << std::endl;
        }
        std::cout << std::endl;
    }
}

inline static void repeatQueryExperiment(char *argv[]) {
    std::cout << "Requested repeatQuery experiment." << std::endl;

    std::string inputFileName(argv[2]);
    std::ifstream inputFile(inputFileName);

    //Read the input text.
    std::string inputText;
    readRemainingFileContents(inputFile, inputText);
    std::cout << "Original input has length " << inputText.length() << std::endl;

    for (int inputLength : queryInputTextLengths) {
        std::string prefix = getPrefix(inputText, inputLength);
        SuffixTree::SuffixTree<CharType, Debug> stree(prefix.c_str(), prefix.length());

        //Run the queries.
        size_t startPosition, length;
        Helpers::Timer queryTimer;
        queryTimer.restart();
        Query::RepeatQuery<CharType, Sentinel, Query::RepeatNoProfiler, Debug> query(&stree);
        std::tie(startPosition, length) = query.runQuery();
        size_t queryTime = queryTimer.getMilliseconds();

        std::cout << "RESULT algo=repeatQueryExperiment"
                  << " queryTime=" << queryTime
                  << " inputType=" << argv[3]
                  << " inputLength=" << prefix.length()
                  << " file=" << inputFileName << std::endl;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cout << "Wrong number of arguments, expecting 2 arguments." << std::endl;
        return 1;
    }

    std::string queryChoice(argv[1]);
    if (queryChoice.compare("topk") == 0) {
        handleTopKQuery(argv);
    } else if (queryChoice.compare("repeat") == 0) {
        handleRepeatQuery(argv);
    } else if (queryChoice.compare("preprocessingExperiment") == 0) {
        preprocessingExperiment(argv);
    } else if (queryChoice.compare("topKQueryExperiment") == 0) {
        topKQueryExperiment(argv);
    } else  if (queryChoice.compare("repeatQueryExperiment") == 0) {
        repeatQueryExperiment(argv);
    } else {
        std::cout << "Unknown query choice." << std::endl;
        return 1;
    }

    return 0;
}
