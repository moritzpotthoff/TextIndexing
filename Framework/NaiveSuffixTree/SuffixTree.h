#pragma once

#include <iostream>
#include <bits/stdc++.h>

#include "Node.h"

namespace UkkonenSuffixTree {

    template<typename CHAR_TYPE, bool DEBUG = false>
    class SuffixTree {
        using CharType = CHAR_TYPE;
        static const bool Debug = DEBUG;

    public:
        SuffixTree(const CharType* input, int n) :
                text(input),
                root(NULL, 0, 0, NULL),
                currentEnd(0),
                n(n) {
        }

        inline void print() {
            std::cout << std::endl;
            std::cout << "    Printing suffix tree: " << std::endl;
            root.print(4);
            std::cout << std::endl;
        }

    private:
        const CharType* text;
        Node<CharType> root;
        int currentEnd;
        int n;//input length
        bool usedRule3;
        Node<CharType>* previouslyInsertedNode;
        Node<CharType>* previousExtensionNode;
        int previousExtensionEndIndex;
    };

}
