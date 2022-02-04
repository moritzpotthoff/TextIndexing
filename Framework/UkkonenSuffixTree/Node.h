#pragma once

#include <map>
#include "Helpers.h"

namespace SuffixTree {
    /**
     * Represents a node in the suffix tree.
     *
     * The structure is as follows:
     *
     *                 [startIndex, endIndex)                map: initial character --> child
     *    [parent]  ----------------------------> [this] --------------------------------------->* children
     *
     * where [startIndex, endIndex) is the substring represented by the edge into this node.
     * The parent pointer is omitted because it is never used.
     *
     * Each node has numberOfLeaves, stringDepth and representedSuffix as preparation for the queries.
     */
    template<typename CHAR_TYPE>
    class Node {
        using CharType = CHAR_TYPE;

    public:
        /**
         * Generate a new node with the given entries.
         */
        Node(int startIndex, int* endIndex, Node<CharType>* suffixLink = NULL) :
                startIndex(startIndex),
                endIndex(endIndex),
                suffixLink(suffixLink),
                numberOfLeaves(0),
                stringDepth(0),
                representedSuffix(0) {
        }

        /**
         * Adds value as a new child for initial character key and ensures that the child's parent pointer is correct.
         */
        inline void addChild(CharType key, Node* value) noexcept {
            children[key] = value;
        }

        /**
         * Returns the node for the given initial character of an outgoing edge.
         */
        inline Node<CharType>* getChild(CharType key) const noexcept {
            auto child = children.find(key);
            if (child == children.end()) return NULL;
            //std::cout << "xxxxx found child: " << child->first << ", " << child->second << std::endl;
            return child->second;
        }

        /**
         * The length of the substring along the edge that enters this node.
         */
        inline int getSubstringLength() const noexcept {
            return *endIndex - startIndex;
        }

        /**
         * Checks if this is an inner node.
         */
        inline bool hasChildren() const noexcept {
            return !children.empty();
        }

        /**
         * Prints the subtree rooted at this node. Used for debugging.
         */
        inline void print(int depth) const noexcept {
            std::cout << std::string(depth, ' ') << "Node " << this << " [" << startIndex << ", " << *endIndex << "), suffixLink " << suffixLink << std::endl;
            for (std::pair<CharType, Node<CharType>*> element : children) {
                std::cout << std::string(depth + 2, ' ') << "Key " << element.first << " is child " << element.second << std::endl;
                element.second->print(depth + 4);
            }
        }

        /**
         * Compactly prints the subtree rooted at this node. Used for debugging.
         */
        inline void printSimple(int depth) const noexcept {
            std::cout << " [" << startIndex << ", " << *endIndex << "), numberOfLeaves: " << numberOfLeaves << ", stringDepth: " << stringDepth << ", representedSuffix: " << representedSuffix << std::endl;
            for (std::pair<CharType, Node<CharType>*> element : children) {
                std::cout << std::string(depth + 4, ' ') << element.first << ": ";
                element.second->printSimple(depth + 4);
            }
        }



    public:
        int startIndex;//inclusive
        int* endIndex;//exclusive, pointer to the int to allow simple increments during Ukkonen's algorithm.
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

