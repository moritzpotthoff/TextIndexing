#pragma once

#include <map>

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
     */
    template<typename CHAR_TYPE>
    class Node {
        using CharType = CHAR_TYPE;

    public:
        /**
         * Generate a new node with the given entries.
         */
        Node(Node<CharType>* parent, size_t startIndex, size_t endIndex) :
                parent(parent),
                startIndex(startIndex),
                endIndex(endIndex),
                numberOfLeaves(0) {
        }

        /**
         * Adds value as a new child for initial character key and ensures that the child's parent pointer is correct.
         */
        inline void addChild(CharType key, Node* value) {
            value->parent = this;
            children[key] = value;
        }

        /**
         * Returns the node for the given initial character of an outgoing edge.
         */
        inline Node<CharType>* getChild(CharType key) {
            return children[key];
        }

        /**
         * The length of the substring along the edge that enters this node.
         */
        inline int getSubstringLength() {
            return endIndex - startIndex;
        }

        /**
         * Checks if this is an inner node.
         */
        inline bool hasChildren() {
            return !children.empty();
        }

        /**
         * Prints the subtree rooted at this node. Used for debugging.
         */
        inline void print(int depth) {
            std::cout << "Node [" << startIndex << ", " << endIndex << "), #leaves=" << numberOfLeaves << ", depth=" << stringDepth << ", representing suffix " << representedSuffix << std::endl;
            for (std::pair<CharType, Node<CharType>*> element : children) {
                std::cout << std::string(depth + 3, ' ') << "Char='" << element.first << "': ";
                element.second->print(depth + 4);
            }
        }

    public:
        //This node's parent
        Node<CharType>* parent;
        //Start index of the substring along the edge, inclusive, absolute.
        size_t startIndex;
        //End index of the substring along the edge, exclusive, absolute.
        size_t endIndex;
        //(Sorted) map of children, identified by the first character along their edge.
        std::map<CharType, Node<CharType>*> children;
        //Number of leaves in the subtree rooted at this node. Only used by queries.
        size_t numberOfLeaves;
        //String depth of this node (including its own incoming edge). Only used by queries.
        size_t stringDepth;
        //For leaves, the suffix that this node represents. For inner nodes in queries, the suffix that one of its leaves represents.
        size_t representedSuffix;
    };

}

