#pragma once

#include <iostream>
#include <bits/stdc++.h>

#include "Node.h"

namespace UkkonenSuffixTree {

    template<typename CHAR_TYPE, bool DEBUG = false>
    class SuffixTree {
        using CharType = CHAR_TYPE;
        static const bool Debug = DEBUG;
        static const int CurrentEndFlag = INT_MAX;
        static const CharType NoEdge = 127;//TODO fix

    public:
        SuffixTree(const CharType* input, int n) :
                text(input),
                root(NULL, 0, 0, NULL),
                currentEnd(0),
                n(n),
                activeEdgeIndex(0),
                activeLength(0),
                activeNode(NULL),
                lastNewInternalNode(NULL),
                remaining(0) {
            if constexpr (Debug) std::cout << "Created new suffix tree with length " << n << std::endl;
            root.parent = &root;
            activeNode = &root;

            createSuffixTree();
        }

        inline void createSuffixTree() noexcept {
            for (size_t i = 0; i < n; i++) { //start new phase for suffix i
                std::cout << "********* Starting phase " << i << std::endl;
                lastNewInternalNode = NULL; //reset, only valid per phase
                currentEnd++; //automatically extend everything
                remaining++; //mark that one more suffix must be added

                while (remaining > 0) {
                    std::cout << "-------- RUNNING NEW SUFFIX IN ITERATION " << i << std::endl;
                    print();
                    if (activeLength == 0) {
                        std::cout << "Active length is 0, start search at activeNode " << activeNode << std::endl;
                        // active length 0, start search for text[i] at root
                        Node<CharType>* child = activeNode->getChild(text[i]);
                        if (child != NULL) {
                            std::cout << "  Active node has child for new character, update active point and break" << std::endl;
                            //activeNode has child for text[i], update active point
                            activeEdgeIndex = child->startIndex;
                            activeLength++;
                            break; //extended by rule three, that ends this phase
                        } else {
                            std::cout << "  Active node has no matching edge, create new leaf." << std::endl;
                            //activeNode has no matching edge, create new leaf with current character as child from activeNode
                            Node<CharType>* newLeaf = new Node<CharType>(activeNode, i, -1);
                            activeNode->addChild(text[i], newLeaf);
                            remaining--;//suffix was inserted
                            std::cout << "  New leaf " << newLeaf << " created" << std::endl;
                        }
                    } else {
                        // active length > 0, character must be found at the current active position
                        CharType nextCharacter = getActivePointCharacter(i); //get next character that would be found after the current active position
                        std::cout << "  Active length not 0, start search. Next character is " << nextCharacter << std::endl;

                        if (nextCharacter == text[i]) { // the next character is a match
                            std::cout << "    Next character is a match, walk down and break" << std::endl;
                            if (lastNewInternalNode != NULL) {
                                lastNewInternalNode->suffixLink = activeNode;
                            }
                            walkDown(i);
                            break;
                        } else {
                            std::cout << "    next character does not match, split edge." << std::endl;
                            //next character does not match, rule 2, add new internal node and split edge accordingly.
                            const int activePointIndex = getActivePointIndex(i);
                            Node<CharType>* activeTarget = getActiveTarget();
                            Node<CharType>* newInternalNode = new Node<CharType>(activeNode, activeTarget->startIndex, activeTarget->startIndex + activeLength);//TODO check if +1 needed
                            Node<CharType>* newLeaf = new Node<CharType>(newInternalNode, i, -1);//leaf that ends as before
                            activeNode->addChild(text[activeEdgeIndex], newInternalNode);
                            newInternalNode->addChild(text[i], newLeaf); //the new leaf starts with the next character that could not be matched
                            newInternalNode->addChild(nextCharacter, activeTarget);// the edge into the old target of the activeEdge starts with the character that did not match the new character
                            activeTarget->startIndex = activePointIndex;
                            if (lastNewInternalNode != NULL) {
                                lastNewInternalNode->suffixLink = newInternalNode;
                            }
                            lastNewInternalNode = newInternalNode;
                            newInternalNode->suffixLink = &root;
                            if (activeNode != &root) {
                                activeNode = activeNode->suffixLink;
                            } else {
                                activeEdgeIndex++;
                                activeLength--;
                            }
                            remaining--;// suffix was inserted
                        }
                    }
                    std::cout << "-------- done with suffix" << std::endl;
                }
                std::cout << "******** Done with iteration " << i << std::endl;
            }
        }

        inline void walkDown(int i) {
            Node<CharType>* activeTarget = getActiveTarget();
            const int activeEdgeSubstringLength = activeTarget->getSubstringLength(currentEnd);
            if (activeEdgeSubstringLength < activeLength) {
                activeNode = activeTarget;
                activeLength -= activeEdgeSubstringLength;
                activeEdgeIndex = activeTarget->getChild(text[i])->startIndex;
            } else {
                activeLength++;//TODO why?
            }
        }

        inline Node<CharType>* getActiveTarget() const noexcept {
            Node<CharType>* target = activeNode->getChild(text[activeEdgeIndex]);
            //AssertMsg(target != NULL, "Active target does not exist.");
            //std::cout << "   internally found active target " << target << std::endl;
            return target;
        }

        inline int getActivePointIndex(int i) noexcept {
            Node<CharType>* activeTarget = getActiveTarget();
            //std::cout << "yyyyy found active target " << activeTarget << std::endl;
            const int activeEdgeStringLength = activeTarget->getSubstringLength(currentEnd);
            if (activeLength < activeEdgeStringLength) {// edge is sufficiently long, return from here
                return activeNode->getActivePointIndex(text[activeEdgeIndex], activeLength, currentEnd);
            }
            if (activeEdgeStringLength == activeLength - 1) {// activeLength
                if (activeTarget->getChild(text[i]) != NULL) {
                    return i;
                }
                //exception
                return -1;
            } else {
                //active edge is not long enough, skip it
                activeNode = activeTarget;
                activeLength = activeLength - activeEdgeStringLength;
                activeEdgeIndex = activeEdgeIndex + activeEdgeStringLength + 1;
                //std::cout << "Search again." << std::endl;
                //std::cout << "    Active node: " << activeNode << std::endl;
                //std::cout << "    Active edge index: " << activeEdgeIndex << " (" << text[activeEdgeIndex] << ")" << std::endl;
                //std::cout << "    Active length: " << activeLength << std::endl;
                return getActivePointIndex(i);
            }
        }

        inline CharType getActivePointCharacter(int i) noexcept {
            return text[getActivePointIndex(i)];
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

    private:
        const CharType* text;
        Node<CharType> root;
        int currentEnd;
        int n;//input length
        int activeEdgeIndex;
        int activeLength;
        Node<CharType>* activeNode;
        Node<CharType>* lastNewInternalNode;
        int remaining;
    };

}
