#pragma once

#include <iostream>
#include <bits/stdc++.h>

#include "Node.h"

namespace SuffixTree {

    template<typename CHAR_TYPE, bool DEBUG = false>
    class SuffixTree {
        using CharType = CHAR_TYPE;
        static const bool Debug = DEBUG;

    public:
        /**
         * THIS CAN BE IGNORED.
         *
         * This is a naive O(n^2) variant of suffix tree construction that I used earlier.
         */
        SuffixTree(const CharType* input, size_t n) :
            text(input),
            root(NULL, 0, 0),
            n(n) {
            if constexpr (Debug) std::cout << "Constructing naive suffix tree for input " << text << std::endl;
            for (size_t i = 0; i < n; i++) {//for each suffix, naively add that to the growing suffix tree
                //match current suffix text[i..n-1] with text starting from root
                bool suffixInserted = false;
                Node<CharType>* currentNode = &root;
                Node<CharType>* child;
                size_t currentStart = i;
                size_t currentSectionStart = i;
                while ((child = currentNode->getChild(text[currentStart]))) {
                    size_t currentEdgeStart = child->startIndex;
                    while (currentEdgeStart < child->endIndex && text[currentEdgeStart] == text[currentStart]) {
                        currentEdgeStart++;
                        currentStart++;
                    }
                    //matched the text until before currentStart using the child's edge.
                    if (currentEdgeStart == child->endIndex && currentEdgeStart > 0 && text[currentEdgeStart - 1] == text[currentStart - 1]) {
                        //matched until the child, set child as new currentNode and continue search. No need to split.
                        currentNode = child;
                        currentSectionStart = currentStart;
                    } else {
                        //matching failed somewhere within the edge from currentNode to child. Have to split the edge and insert a new node.

                        //Add new splitter node that has child's parent and the appropriate edge section.
                        Node<CharType>* splitterNode = new Node(currentNode, child->startIndex, currentEdgeStart);

                        //Adjust the incoming edge of child  accordingly.
                        child->parent = splitterNode;
                        child->startIndex = splitterNode->endIndex;

                        //add new node that represents the remaining part of the suffix starting at currentStart and ending at n
                        Node<CharType>* newNode = new Node(splitterNode, currentStart, n);
                        newNode->representedSuffix = i;

                        //set correct children for splitter
                        splitterNode->addChild(text[currentStart], newNode);
                        splitterNode->addChild(text[currentEdgeStart], child);

                        //mark splitter as child of currentNode
                        currentNode->children[text[currentSectionStart]] = splitterNode;

                        //done with the suffix, break
                        suffixInserted = true;
                        break;
                    }
                }

                if (!suffixInserted) {
                    //no suitable edge found from currentNode, add edge from there.
                    Node<CharType>* newNode = new Node(currentNode, currentStart, n);
                    newNode->representedSuffix = i;
                    currentNode->addChild(text[currentStart], newNode);
                }

                if constexpr (Debug) {
                    std::cout << " After inserting the current suffix: " << std::endl;
                    root.print(4);
                    std::cout << std::endl << std::endl << std::endl;
                }
            }

            if constexpr (Debug) {
                std::cout << "Done constructing suffix tree: " << std::endl;
                root.print(4);
            }
        }

        inline void print() {
            std::cout << std::endl;
            std::cout << "    Printing suffix tree: " << std::endl;
            root.print(4);
            std::cout << std::endl;
        }

        inline std::string substring(size_t startIndex, size_t length) const noexcept {
            std::string result(text + startIndex);
            result.resize(length);
            return result;
        }

    public:
        const CharType* text;
        Node<CharType> root;
        size_t n;//input length
    };

}
