#include <iostream>
#include <fstream>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "Wrong number of arguments, expecting 2 arguments." << std::endl;
        return 1;
    }

    std::string queryChoice(argv[1]);
    std::string inputFileName(argv[2]);

    std::ifstream inputFile;
    inputFile.open(inputFileName);
    if (!inputFile.is_open()) {
        std::cout << "Error while reading file." << std::endl;
        return 1;
    }
    std::cout << "Reading input file" << std::endl;
    char line[100];
    while (inputFile.getline(line, 100)) {
        std::cout << line << std::endl;
    }
    inputFile.close();

    if (queryChoice.compare("topk") == 0) {
        std::cout << "Requested topk query." << std::endl;
    } else if (queryChoice.compare("repeat") == 0) {
        std::cout << "Requested repeat query." << std::endl;
    } else {
        std::cout << "Unknown query choice." << std::endl;
        return 1;
    }

    return 0;
}
