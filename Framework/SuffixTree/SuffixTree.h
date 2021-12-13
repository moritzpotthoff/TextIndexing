#pragma once

#include <iostream>

#include "Node.h"

namespace SuffixTree {

    template<typename CHAR_TYPE>
    class SuffixTree {
        using CharType = CHAR_TYPE;

    public:
        SuffixTree(std::string input) {
            std::cout << "Created new suffix tree for " << input << std::endl;
        }

    private:
        Node<CharType> root;
    };

}
