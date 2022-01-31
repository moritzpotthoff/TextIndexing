#pragma once

#include <map>
#include "Helpers.h"

namespace SuffixTree {
    template<typename CHAR_TYPE>
    class Node {
        using CharType = CHAR_TYPE;

    public:
        Node(int startIndex, int* endIndex, Node<CharType>* suffixLink = NULL) :
                startIndex(startIndex),
                endIndex(endIndex),
                suffixLink(suffixLink),
                numberOfLeaves(0),
                stringDepth(0),
                representedSuffix(0) {
            //AssertMsg(*endIndex > startIndex || *endIndex == 0, "Wrong node indices");
        }

        inline int getActivePointIndex(CharType activeEdge, int activeLength) noexcept {
            return children[activeEdge]->getTextIndex(activeLength);
        }

        inline int getTextIndex(int offset) const noexcept {
            AssertMsg((offset >= 0 && startIndex + offset < *endIndex), "Offset is invalid.");
            return startIndex + offset;
        }

        inline void addChild(CharType key, Node* value) noexcept {
            children[key] = value;
        }

        inline Node<CharType>* getChild(CharType key) const noexcept {
            auto child = children.find(key);
            if (child == children.end()) return NULL;
            //std::cout << "xxxxx found child: " << child->first << ", " << child->second << std::endl;
            return child->second;
        }

        inline int getSubstringLength() const noexcept {
            return *endIndex - startIndex;
        }

        inline bool hasChildren() const noexcept {
            return !children.empty();
        }

        inline void print(int depth) const noexcept {
            std::cout << std::string(depth, ' ') << "Node " << this << " [" << startIndex << ", " << *endIndex << "), suffixLink " << suffixLink << std::endl;
            for (std::pair<CharType, Node<CharType>*> element : children) {
                std::cout << std::string(depth + 2, ' ') << "Key " << element.first << " is child " << element.second << std::endl;
                element.second->print(depth + 4);
            }
        }

        inline void printSimple(int depth) const noexcept {
            std::cout << " [" << startIndex << ", " << *endIndex << "), numberOfLeaves: " << numberOfLeaves << ", stringDepth: " << stringDepth << ", representedSuffix: " << representedSuffix << std::endl;
            for (std::pair<CharType, Node<CharType>*> element : children) {
                std::cout << std::string(depth + 4, ' ') << element.first << ": ";
                element.second->printSimple(depth + 4);
            }
        }



    public:
        int startIndex;//inclusive
        int* endIndex;//exclusive
        Node<CharType>* suffixLink;
        std::map<CharType, Node<CharType>*> children;
        //Number of leaves in the subtree rooted at this node. Only used by queries.
        int numberOfLeaves;
        //String depth of this node (including its own incoming edge). Only used by queries.
        int stringDepth;
        //For leaves, the suffix that this node represents. For inner nodes in queries, the suffix that one of its leaves represents.
        int representedSuffix;
    };

}

