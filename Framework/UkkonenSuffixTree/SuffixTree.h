#pragma once

#include <iostream>
#include <bits/stdc++.h>

#include "Node.h"

namespace SuffixTree {

    /**
     * Creates a Suffix Tree using Ukkonen's algorithm.
     *
     * This implementation is based on the following sources:
     *      - https://www.geeksforgeeks.org/ukkonens-suffix-tree-construction-part-1/ etc.
     *      - https://stackoverflow.com/questions/9452701/ukkonens-suffix-tree-algorithm-in-plain-english/9513423#9513423
     *      - https://en.wikipedia.org/wiki/Ukkonen%27s_algorithm
     *      - https://www.youtube.com/watch?v=aPRqocoBsFQ
     *      - https://brenden.github.io/ukkonen-animation/
     *
     * The memory consumption is pretty bad, but I do not have more time to address that :|
     *
     */
    template<typename CHAR_TYPE, bool DEBUG = false>
    class SuffixTree {
        using CharType = CHAR_TYPE;
        static const bool Debug = DEBUG;

    public:
        SuffixTree(const CharType* input, int n) :
                text(input),
                root(0, NULL, NULL),//initialize empty tree
                currentEnd(0),
                n(n),
                activeEdgeIndex(0),//initialize active point
                activeLength(0),
                activeNode(NULL),
                lastNewInternalNode(NULL),
                remaining(0) {
            root.endIndex = new int(0);
            //Initially, all insertions are made from root
            activeNode = &root;

            //Run Ukkonen's algorithm, for each character, run its phase.
            for (int i = 0; i < n; i++) {
                runPhase(i);
            }
            //After the construction, all leaf edges have endIndex currentEnd = n - 1. Since end indices are exclusive, increment them to n.
            currentEnd++;

            if constexpr (Debug) {
                std::cout << std::endl << std::endl << std::endl;
                print();
                std::cout << std::endl << std::endl << std::endl;
                validate();
            }
        }

        /**
         * Runs phase i (for new character text[i]) of Ukkonen's algorithm.
         */
        inline void runPhase(int i) {
            lastNewInternalNode = NULL; //reset, only valid per phase
            currentEnd = i; //automatically extend all existing suffixes
            remaining++; //mark that one more suffix must be added

            //As long as there are suffixes that need to be inserted, insert them.
            while (remaining > 0) {
                //If active length is 0, the insertion has to be made at active node directly.
                //In that case, there needs to be an outgoing edge with character text[i]
                //Adjust the activeEdgeIndex accordingly.
                if (activeLength == 0) {
                    activeEdgeIndex = i;
                }

                //Get the activeTarget, the node that the activeEdge points to
                Node<CharType>* activeTarget = getActiveTarget();
                if (activeTarget == NULL) {
                    //The activeTarget does not exist, i.e., there is no correct outgoing edge from activeNode
                    //Create a new leaf from activeNode

                    //The leaf represents the suffix starting at i and ending at currentEnd, its suffix link is &root by default.
                    Node<CharType>* newLeaf = new Node<CharType>(i, &currentEnd, &root);
                    //Add the new leaf as a child. Since its edge starts with text[activeEdgeIndex], use that as key.
                    activeNode->addChild(text[activeEdgeIndex], newLeaf);

                    //If we have previously inserted a new internal node during this phase, we need to add a suffix link.
                    if (lastNewInternalNode != NULL) {
                        //the previously inserted node points to the new active node so that the active point can be updated efficiently.
                        lastNewInternalNode->suffixLink = activeNode;
                        lastNewInternalNode = NULL;//avoid overwriting the suffix link
                    }
                } else {
                    //The activeTarget does not exist, i.e., there is a correct outgoing edge from activeNode.

                    //Make sure that the active point is correctly represented. For this, walk down the active edge.
                    //If the end of the active edge was passed, go to next iteration (remaining was not changed)
                    //to correctly handle the new active pointer (activeLength might have become 0).
                    if (walkDown(activeTarget)) {
                        continue;
                    }

                    //The active point is at activeTarget->startIndex + activeLength. Check if that character matches text[i].
                    if (text[activeTarget->startIndex + activeLength] == text[i]) {
                        //It's a match, the character is already there. Extend by rule 3.

                        //If activeNode is not &root, set the active node of a previously inserted
                        //internal node to activeNode.
                        if (lastNewInternalNode != NULL && activeNode != &root) {
                            lastNewInternalNode->suffixLink = activeNode;
                            lastNewInternalNode = NULL;//avoid overwriting the suffix link
                        }
                        //We walked along the current edge, increment activeLength accordingly.
                        activeLength++;
                        //Extended by rule 3, no more insertions necessary in this phase.
                        break;
                    } else {
                        //No match, the character is not at the edge, yet. We need to split the edge and insert a new leaf.

                        //Get new int for the end of the edge into the new internal node that will be the splitter.
                        internalNodeEnd = new int();
                        //The edge into the new internal node ends at the active point (exclusively).
                        *internalNodeEnd = activeTarget->startIndex + activeLength;
                        //Create the new internal node, it starts at the same position as the active edge.
                        Node<CharType> *newInternalNode = new Node<CharType>(activeTarget->startIndex, internalNodeEnd, &root);
                        //Create the new leaf. It starts at i and ends at currentEnd.
                        Node<CharType> *newLeaf = new Node<CharType>(i, &currentEnd, &root);
                        //Replace activeTarget by newInternalNode as child for text[activeEdgeIndex] of activeNode
                        activeNode->addChild(text[activeEdgeIndex], newInternalNode);
                        //The splitter has two children: newLeaf for text[i] and the old activeTarget for the active point text[activeTarget->startIndex + activeLength]
                        newInternalNode->addChild(text[i], newLeaf);
                        newInternalNode->addChild(text[activeTarget->startIndex + activeLength], activeTarget);
                        //The edge into the old activeTarget now starts after the active point.
                        activeTarget->startIndex += activeLength;

                        //We inserted a new internal node. If there was one before, set the suffix link accordingly.
                        if (lastNewInternalNode != NULL) {
                            lastNewInternalNode->suffixLink = newInternalNode;
                        }
                        lastNewInternalNode = newInternalNode;//update for the next iterations
                    }
                }
                //If we are here, one suffix was inserted in the previous iteration with rule 2.
                remaining--;
                //Update the active point
                if (activeNode == &root && activeLength > 0) {
                    //If active node was root and active length > 0 after the iteration, update active point according to rule 1:
                    activeLength--;
                    //activeEdgeIndex must point to the first character of the next suffix we have to insert
                    activeEdgeIndex = i - remaining + 1;
                } else if (activeNode != &root) {
                    //If activeNode was not root, follow the suffix link to update activeNode.
                    activeNode = activeNode->suffixLink;
                    //activeEdgeIndex and activeLength do not need to be updated since the edges out of the new active
                    //node are the same as before since there is a suffix link.
                }
            }
        }

        /**
         * Walks down the current active point to make sure it is valid. Skip the edge if necessary.
         */
        inline bool walkDown(Node<CharType>* activeTarget) {
            const int activeEdgeSubstringLength = activeTarget->getSubstringLength();
            if (activeLength >= activeEdgeSubstringLength) {
                //activeLength points behind the end of the activeEdge, skip the edge.
                activeNode = activeTarget;
                activeLength -= activeEdgeSubstringLength;
                activeEdgeIndex += activeEdgeSubstringLength;//the new activeEdgeIndex is exactly the length further back
                return true;//an edge was skipped
            }
            return false;//no skip necessary
        }

        /**
         * Returns the node that the active edge points to.
         */
        inline Node<CharType>* getActiveTarget() const noexcept {
            return activeNode->getChild(text[activeEdgeIndex]);
        }

        /**
         * Prints the suffix tree in detail.
         */
        inline void print() noexcept {
            std::cout << std::endl;
            std::cout << "    Printing suffix tree: " << std::endl;
            std::cout << "    Active node: " << activeNode << std::endl;
            std::cout << "    Active edge index: " << activeEdgeIndex << " (" << text[activeEdgeIndex] << ")" << std::endl;
            std::cout << "    Active length: " << activeLength << std::endl;
            std::cout << "    CurrentEnd: " << currentEnd << std::endl;
            std::cout << "    Remaining suffixes: " << remaining << std::endl;
            root.print(4);
            std::cout << std::endl;
        }

        /**
         * Compactly prints the suffix tree.
         */
        inline void printSimple() noexcept {
            root.printSimple(0);
            std::cout << std::endl;
        }

        /**
         * Prints the suffix array that is derived from the suffix tree for validation.
         */
        inline void validate() noexcept {
            std::cout << "SA: ";
            saDfs(&root, 0);
            std::cout << std::endl;
        }

        /**
         * Goes through the tree to generate the suffix array.
         */
        inline void saDfs(Node<CharType>* node, int stringDepth) {
            stringDepth += node->getSubstringLength();
            if (node->hasChildren()) {
                for (const auto &[key, child] : node->children) {
                    saDfs(child, stringDepth);
                }
            } else {
                //leaf
                int suffixIndex = *(node->endIndex) - stringDepth;
                std::cout << suffixIndex << " ";
            }
        }

        /**
         * Returns the substring of the input text with length length starting at startIndex.
         */
        inline std::string substring(size_t startIndex, size_t length) const noexcept {
            std::string result(text + startIndex);
            result.resize(length);
            return result;
        }

    public:
        //The input text
        const CharType* text;
        //Root of the tree
        Node<CharType> root;
        //The index that all leaf-edges currently end at.
        int currentEnd;
        //Input length
        int n;
        //The index of the character in the text that the active edge starts with from activeNode
        int activeEdgeIndex;
        //The active length, the index of the active point along the active edge
        int activeLength;
        //The active node, from which the active edge starts
        Node<CharType>* activeNode;
        //The last created internal node, used to correctly set suffix links.
        Node<CharType>* lastNewInternalNode;
        //The number of suffixes that still need to be inserted
        int remaining;
        //End of the edge for new internal nodes, needed because all end indices are pointers to ints.
        int* internalNodeEnd;
    };
}