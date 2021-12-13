#pragma once

#include <map>

namespace SuffixTree {
    template<typename CHAR_TYPE>
    class Node {
        using CharType = CHAR_TYPE;

    public:
        Node(Node *parent = NULL, int startIndex = 0, int endIndex = 0, Node *suffixLink = NULL) :
                parent(parent),
                startIndex(startIndex),
                endIndex(endIndex),
                suffixLink(suffixLink) {
        }

    private:
        Node* parent;
        int startIndex;
        int endIndex;
        Node* suffixLink;
        std::map<CharType, Node*> children;
    };

}

