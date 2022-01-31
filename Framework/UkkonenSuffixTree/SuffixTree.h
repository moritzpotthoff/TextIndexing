#pragma once

#include <iostream>
#include <bits/stdc++.h>

#include "Node.h"

namespace SuffixTree {

    /**
     * Creates a Suffix Tree using Ukkonen's algorithm.
     */
    template<typename CHAR_TYPE, bool DEBUG = false>
    class SuffixTree {
        using CharType = CHAR_TYPE;
        static const bool Debug = DEBUG;

    public:
        SuffixTree(const CharType* input, int n) :
                text(input),
                root(NULL, 0, NULL, NULL),
                currentEnd(0),
                n(n),
                activeEdgeIndex(0),
                activeLength(0),
                activeNode(NULL),
                lastNewInternalNode(NULL),
                remaining(0) {
            if constexpr (Debug) std::cout << "Created new suffix tree with length " << n << std::endl;
            root.endIndex = new int(0);
            root.parent = &root;
            activeNode = &root;

            createSuffixTree();

            if constexpr (Debug) {
                std::cout << std::endl << std::endl << std::endl;
                print();
                std::cout << std::endl << std::endl << std::endl;
                validate();
            }
        }

        /**
         * Creates a suffix tree using Ukkonen's algorithm for the given input.
         */
        inline void createSuffixTree() noexcept {
            for (size_t i = 0; i < n; i++) { //start new phase for suffix i
                runPhase(i);
            }
            currentEnd++; //make sure ends are inclusive.
        }

        /**
         * Runs phase i (for new character text[i]) of Ukkonen's algorithm.
         */
        inline void runPhase(int i) {
            if constexpr (Debug)std::cout << "********* Starting phase " << i << std::endl;
            lastNewInternalNode = NULL; //reset, only valid per phase
            currentEnd = i; //automatically extend all existing suffixes
            remaining++; //mark that one more suffix must be added

            //As long as there are suffixes that need to be inserted, insert them
            while (remaining > 0) {
                if constexpr (Debug) std::cout << "-------- RUNNING NEW SUFFIX IN ITERATION " << i << std::endl;
                if constexpr (Debug) print();
                if (activeLength == 0) {
                    activeEdgeIndex = i;
                }

                Node<CharType>* activeTarget = getActiveTarget();
                if (activeTarget == NULL) {
                    //there is no correct outgoing edge from activeNode, create new leaf
                    Node<CharType>* newLeaf = new Node<CharType>(activeNode, i, &currentEnd, &root);
                    activeNode->addChild(text[activeEdgeIndex], newLeaf);

                    if (lastNewInternalNode != NULL) {
                        lastNewInternalNode->suffixLink = activeNode;
                        lastNewInternalNode = NULL;
                    }
                } else {
                    //there is a correct outgoing edge from activeNode
                    if (walkDown(activeTarget)) {
                        continue;
                    }

                    if (text[activeTarget->startIndex + activeLength] == text[i]) {
                        //character is already there
                        if (lastNewInternalNode != NULL && activeNode != &root) {
                            lastNewInternalNode->suffixLink = activeNode;
                            lastNewInternalNode = NULL;
                        }
                        activeLength++;
                        break;
                    }

                    splitIndex = new int();
                    *splitIndex = activeTarget->startIndex + activeLength;
                    Node<CharType> *newInternalNode = new Node<CharType>(activeNode, activeTarget->startIndex, splitIndex, &root);
                    Node<CharType> *newLeaf = new Node<CharType>(newInternalNode, i, &currentEnd, &root);//leaf that ends as before
                    activeNode->addChild(text[activeEdgeIndex], newInternalNode);
                    newInternalNode->addChild(text[i], newLeaf); //the new leaf starts with the next character that could not be matched
                    newInternalNode->addChild(text[activeTarget->startIndex + activeLength], activeTarget);// the edge into the old target of the activeEdge starts with the character that did not match the new character
                    activeTarget->startIndex += activeLength;
                    if (lastNewInternalNode != NULL) {
                        lastNewInternalNode->suffixLink = newInternalNode;
                    }
                    lastNewInternalNode = newInternalNode;
                }
                remaining--;
                if (activeNode == &root && activeLength > 0) {
                    activeLength--;
                    activeEdgeIndex = i - remaining + 1;
                } else if (activeNode != &root) {
                    activeNode = activeNode->suffixLink;
                }


                if constexpr (Debug)std::cout << "-------- done with suffix" << std::endl;
            }
            if constexpr (Debug)std::cout << "******** Done with iteration " << i << std::endl;
        }

        /**
         * Walks down the current active point to make sure it is valid
         */
        inline bool walkDown(Node<CharType>* activeTarget) {
            const int activeEdgeSubstringLength = activeTarget->getSubstringLength();
            if (activeLength >= activeEdgeSubstringLength) {
                activeNode = activeTarget;
                activeLength -= activeEdgeSubstringLength;
                activeEdgeIndex += activeEdgeSubstringLength;
                return true;
            }
            return false;
        }

        inline Node<CharType>* getActiveTarget() const noexcept {
            return activeNode->getChild(text[activeEdgeIndex]);
        }

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

        inline void printSimple() noexcept {
            root.printSimple(0);
            std::cout << std::endl;
        }

        inline void validate() noexcept {
            std::cout << "SA: ";
            saDfs(&root, 0);
            std::cout << std::endl;
        }

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

        inline void printingDfs(Node<CharType>* node, int depth) {
            std::cout << std::string(depth, ' ');
            for (int l = 0; l < node->getSubstringLength(); l++) {
                std::cout << text[node->startIndex + l];
            }
            std::cout << std::endl;
            for (const auto & [key, child] : node->children) {
                dfs(child, depth + node->getSubstringLength());
            }
        }

        inline std::string substring(size_t startIndex, size_t length) const noexcept {
            std::string result(text + startIndex);
            result.resize(length);
            return result;
        }

    public:
        const CharType* text;
        Node<CharType> root;
        int currentEnd;
        int n;//input length
        int activeEdgeIndex;
        int activeLength;
        Node<CharType>* activeNode;
        Node<CharType>* lastNewInternalNode;
        int remaining;
        int* splitIndex;
    };
}