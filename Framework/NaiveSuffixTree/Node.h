#pragma once

#include <map>

namespace NaiveSuffixTree {
    template<typename CHAR_TYPE>
    class Node {
        using CharType = CHAR_TYPE;
        static const CharType NoEdge = 127;//TODO fix

    public:
        Node(Node<CharType>* parent, size_t startIndex, size_t endIndex) :
                parent(parent),
                startIndex(startIndex),
                endIndex(endIndex) {
        }

        inline void addChild(CharType key, Node* value) {
            value->parent = this;
            children[key] = value;
        }

        inline Node<CharType>* getChild(CharType key) {
            return children[key];
        }

        inline int getSubstringLength() {
            return endIndex - startIndex;
        }

        inline bool hasChildren() {
            return !children.empty();
        }

        inline void print(int depth) {
            std::cout << "Node [" << startIndex << ", " << endIndex << "), #leaves=" << numberOfLeaves << ", depth=" << previousStringDepth << std::endl;
            for (std::pair<CharType, Node<CharType>*> element : children) {
                std::cout << std::string(depth + 3, ' ') << "Char='" << element.first << "': ";
                element.second->print(depth + 4);
            }
        }

    public:
        Node<CharType>* parent;
        size_t startIndex;//inclusive, absolute index
        size_t endIndex;//exclusive, absolute index
        std::map<CharType, Node<CharType>*> children;
        size_t numberOfLeaves;
        size_t previousStringDepth;
    };

}

