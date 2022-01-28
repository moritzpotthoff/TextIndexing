#pragma once

#include <iostream>
#include <bits/stdc++.h>

#include "Node.h"

namespace UkkonenSuffixTree {

    /**
     * Creates a Suffix Tree using Ukkonen's algorithm.
     */
    template<typename CHAR_TYPE, bool DEBUG = false>
    class SuffixTree {
        using CharType = CHAR_TYPE;
        static const bool Debug = DEBUG;
        static const int CurrentEndFlag = -1; //marks that an edges end is actually the global currentEnd

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

            std::cout << std::endl << std::endl << std::endl;
            print();
            std::cout << std::endl << std::endl << std::endl;
            validate();
        }

        /**
         * Creates a suffix tree using Ukkonen's algorithm for the given input.
         */
        inline void createSuffixTree() noexcept {
            for (size_t i = 0; i < n; i++) { //start new phase for suffix i
                runPhase(i);
            }
        }

        /**
         * Runs phase i (for new character text[i]) of Ukkonen's algorithm.
         */
        inline void runPhase(int i) {
            std::cout << "********* Starting phase " << i << std::endl;
            lastNewInternalNode = NULL; //reset, only valid per phase
            currentEnd++; //automatically extend all existing suffixes
            remaining++; //mark that one more suffix must be added

            //As long as there are suffixes that need to be inserted, insert them
            while (remaining > 0) {
                walkDown(i);
                std::cout << "-------- RUNNING NEW SUFFIX IN ITERATION " << i << std::endl;
                print();
                if (activeLength == 0) {
                    // active length 0, start search for text[i] at activeNode
                    std::cout << "Active length is 0, start search at activeNode " << activeNode << std::endl;
                    Node<CharType>* child = activeNode->getChild(text[i]);
                    if (child != NULL) {
                        std::cout << "  Active node has child for new character, update active point and break" << std::endl;
                        //activeNode has child for text[i], update active point
                        activeEdgeIndex = i;
                        activeLength++;
                        walkDown(i);
                        break; //extended by rule three, that ends this phase
                    } else {
                        std::cout << "  Active node has no matching edge, create new leaf." << std::endl;
                        //activeNode has no matching edge, create new leaf with current character as child from activeNode
                        Node<CharType>* newLeaf = new Node<CharType>(activeNode, i, CurrentEndFlag);
                        /*if (!activeNode->hasChildren()) {
                            //was leaf, now got internal node
                            if (lastNewInternalNode != NULL) {
                                lastNewInternalNode->suffixLink = activeNode;
                            }
                            lastNewInternalNode = activeNode;
                        }*/
                        activeNode->addChild(text[i], newLeaf);
                        remaining--;//suffix was inserted
                        std::cout << "  New leaf " << newLeaf << " created" << std::endl;
                    }
                } else {
                    // active length > 0, character must be found at the current active position
                    const int activePointIndex = getActivePointIndex(i);
                    bool sufficientPath = (activePointIndex != -1);//get next character that would be found after the current active position

                    if (!sufficientPath) {
                        //active point is beyond the end of the path, need to add a new leaf to the path
                        Node<CharType>* activeTarget = getActiveTarget();
                        Node<CharType>* newLeaf = new Node<CharType>(activeTarget, i, CurrentEndFlag);
                        activeTarget->addChild(text[i], newLeaf);

                        if (lastNewInternalNode != NULL) {
                            lastNewInternalNode->suffixLink = getActiveTarget();
                        }
                        lastNewInternalNode = activeTarget;

                        if (activeNode != &root) {
                            activeNode = activeNode->suffixLink;
                        } else {
                            activeEdgeIndex++;
                            activeLength--;
                        }
                        remaining--;
                    } else {
                        CharType nextCharacter = text[activePointIndex];
                        std::cout << "  Active length not 0, start search. Next character is " << nextCharacter << std::endl;

                        if (nextCharacter == text[i]) { // the next character is a match
                            std::cout << "    Next character is a match, walk down and break" << std::endl;
                            if (lastNewInternalNode != NULL) {
                                lastNewInternalNode->suffixLink = getActiveTarget();
                            }
                            activeLength++;
                            walkDown(i);
                            break;
                        } else {
                            std::cout << "    next character does not match, split edge." << std::endl;

                            if (activeLength == 0) {
                                AssertMsg(false, "This should never happen. ActiveLength == 0");
                            }
                            /*
                                std::cout << "XYZXYZXYZ Active length is 0, start search at activeNode " << activeNode << std::endl;
                                Node<CharType>* child = activeNode->getChild(text[i]);
                                if (child != NULL) {
                                    std::cout << "  Active node has child for new character, update active point and break" << std::endl;
                                    //activeNode has child for text[i], update active point
                                    activeEdgeIndex = child->startIndex;
                                    activeLength++;
                                    walkDown(i);
                                    break; //extended by rule three, that ends this phase
                                } else {
                                    std::cout << "  Active node has no matching edge, create new leaf." << std::endl;
                                    //activeNode has no matching edge, create new leaf with current character as child from activeNode
                                    Node<CharType>* newLeaf = new Node<CharType>(activeNode, i, CurrentEndFlag);

                                    activeNode->addChild(text[i], newLeaf);
                                    remaining--;//suffix was inserted
                                    std::cout << "  New leaf " << newLeaf << " created" << std::endl;
                                }
                                activeNode = &root;
                                activeLength = 0;
                                activeEdgeIndex = 0;
                            */

                            //next character does not match, rule 2, add new internal node and split edge accordingly.
                            std::cout << "    WWWWWWWWWWWWWWWWWW before:" << std::endl;
                            print();
                            std::cout << "    WWWWWWWWWWWWWWWWWW active point index :" << activePointIndex << std::endl;

                            Node<CharType> *activeTarget = getActiveTarget();
                            Node<CharType> *newInternalNode = new Node<CharType>(activeNode, activeTarget->startIndex, activePointIndex);//TODO check if +1 needed
                            Node<CharType> *newLeaf = new Node<CharType>(newInternalNode, i, CurrentEndFlag);//leaf that ends as before
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
                            std::cout << "    WWWWWWWWWWWWWWWWWW after:" << std::endl;
                            print();
                        }
                    }
                }
                std::cout << "-------- done with suffix" << std::endl;
            }
            std::cout << "******** Done with iteration " << i << std::endl;
        }

        /**
         * Walks down the current active point to make sure it is valid
         */
        inline void walkDown(int i) {
            Node<CharType>* activeTarget = getActiveTarget();
            if (activeTarget == NULL) return;
            const int activeEdgeSubstringLength = activeTarget->getSubstringLength(currentEnd);
            if (activeEdgeSubstringLength < activeLength) {
                activeNode = activeTarget;
                activeLength -= activeEdgeSubstringLength;
                activeEdgeIndex = i;
                //walkDown(i);
            }
        }

        inline Node<CharType>* getActiveTarget() const noexcept {
            return activeNode->getChild(text[activeEdgeIndex]);
        }

        inline int getActivePointIndex(int i) noexcept {
            //TODO make this non-recursive
            Node<CharType>* activeTarget = getActiveTarget();
            if (activeTarget == NULL) return -1;
            const int activeEdgeStringLength = activeTarget->getSubstringLength(currentEnd);
            if (activeLength < activeEdgeStringLength) {// edge is sufficiently long, return from here
                std::cout << " edge sufficiently long, return from there" << std::endl;
                return activeNode->getActivePointIndex(text[activeEdgeIndex], activeLength, currentEnd);
            }
            if (activeEdgeStringLength == activeLength) {// activeLength
                std::cout << " edge length just correct" << std::endl;
                if (activeTarget->getChild(text[i]) != NULL) {
                    return i;
                }
                std::cout << " FAIL" << std::endl;
                return -1;
            } else {
                AssertMsg(false, "This should never happen. ActiveLength too large");
                /*
                std::cout << " skip edge" << std::endl;
                //active edge is not long enough, skip it
                activeNode = activeTarget;
                activeLength -= activeEdgeStringLength;
                activeEdgeIndex = i;
                return getActivePointIndex(i);
                 */
            }
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

        inline void validate() noexcept {
            std::cout << "SA: ";
            saDfs(&root, 0);
            std::cout << std::endl;
        }

        inline void saDfs(Node<CharType>* node, int stringDepth) {
            stringDepth += node->getSubstringLength(currentEnd);
            if (node->hasChildren()) {
                for (const auto &[key, child] : node->children) {
                    saDfs(child, stringDepth);
                }
            } else {
                //leaf
                int suffixIndex = node->trueEndIndex(currentEnd) - stringDepth;
                std::cout << suffixIndex << " ";
            }
        }

        inline void printingDfs(Node<CharType>* node, int depth) {
            std::cout << std::string(depth, ' ');
            for (int l = 0; l < node->getSubstringLength(currentEnd); l++) {
                std::cout << text[node->startIndex + l];
            }
            std::cout << std::endl;
            for (const auto & [key, child] : node->children) {
                dfs(child, depth + node->getSubstringLength(currentEnd));
            }
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