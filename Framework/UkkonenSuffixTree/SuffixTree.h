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
            n(n) {
            if constexpr (Debug) std::cout << "Created new suffix tree with length " << n << std::endl;

            root.setParent(&root);
            //First phase: add edge from root to new node for first character.
            if constexpr (Debug) std::cout << "Running initial phase for new character: '" << text[0] << "'." << std::endl;
            Node<CharType>* firstChild = new Node<CharType>(&root, 0, CurrentEndFlag, NULL);
            root.addChild(text[0], firstChild);

            print();

            //Initialize variables for extensions
            previouslyInsertedNode = firstChild;
            previousExtensionNode = firstChild;
            previousExtensionEndIndex = 1;//ends at 1 because one letter comes after edge to firstChild

            //Iterate over the remaining text, for each new character, add the necessary explicit suffixes in the new phase.
            int previousEnd = 1;
            for (int phase = 1; phase < n; ++phase) {
                if constexpr (Debug) std::cout << "Running phase " << phase << " for new character: '" << text[phase] << "'." << std::endl;
                //Update currentEnd index to automatically extend all existing edges. This handles all implicit extensions.
                currentEnd = phase;

                //Do the necessary extensions
                for (int j = previousEnd + 1; j <= phase + 1; j++) {
                    singleExtension(phase, j);
                    if (usedRule3) {
                        usedRule3 = false;
                        break;
                    }
                    previousEnd++;
                }

                print();
            }
        }

        inline void singleExtension(int i, int j) {
            if constexpr (Debug) std::cout << " Running extension " << j << std::endl;

            //1. Find node v and remember the string that was used
            int gammaStart, gammaEnd;
            Node<CharType>* v = previousExtensionNode->walkUp(previousExtensionEndIndex, &root, &gammaStart, &gammaEnd);
            AssertMsg(v != NULL, "Did not find node with walkUp.");

            CharType activeEdge;
            int activeLength;

            if (v == &root) {
                //2.a If v is the root, follow the path for T[j..i] from the root
                std::tie(v, activeEdge, activeLength) = walkDown(v, i - 1, j - 1);
            } else {
                //2.b If v is not the root, traverse suffix link from v to s(v) and walk down from s(v) following the string used in walk up of step 1.
                v = v->getSuffixLink();
                AssertMsg(v != NULL, "Did not find suffix link.");
                std::tie(v, activeEdge, activeLength) = walkDown(v, gammaStart, gammaEnd);
            }

            //3. Use extension rules 2 and 3 from v to make sure that string T[j..i+1] is in the tree.
            CharType newCharacter = text[i];
            Node<CharType>* newNode;
            Node<CharType>* appendToNode;
            if (rule2Applies(v, activeEdge, activeLength, newCharacter)) {
                if constexpr (Debug) std::cout << "  Use rule 2 for extension with newCharacter " << newCharacter << std::endl;
                //Use rule 2.
                Node<CharType>* targetEdge = v->getChild(activeEdge);
                AssertMsg(targetEdge != NULL, "Target edge not found.");
                appendToNode = v;
                if (activeLength > 0 && activeLength < targetEdge->getSubstringLength()) {
                    if constexpr (Debug) std::cout << "  Split for extension" << std::endl;
                    //the active point is within the edge, split accordingly and get the inserted splitter node
                    appendToNode = targetEdge->splitEdge(activeEdge, activeLength, text[targetEdge->getTextIndex(activeLength)]);
                }
                //Actually append a node for the new character.
                newNode = new Node<CharType>(appendToNode, targetEdge->getTextIndex(activeLength) + 1, CurrentEndFlag, NULL);
                appendToNode->addChild(newCharacter, newNode);
            } else {
                if constexpr (Debug) std::cout << "  Use rule 3 for extension, end." << std::endl;
                //Use rule 3.
                usedRule3 = true;
            }

            //4. If necessary, create suffix link from node inserted in the previous extension to the node inserted in this extension.
            if (previouslyInsertedNode != NULL) {
                previouslyInsertedNode->addSuffixLink(newNode);
            }

            //update global values for the next iteration.
            previouslyInsertedNode = appendToNode;
            previousExtensionNode = newNode;
            previousExtensionEndIndex = 1;//ends at 1 because one letter comes after edge to newNode
        }

        inline bool rule2Applies(Node<CharType>* activeNode, CharType activeEdge, int activeLength, CharType newCharacter) {
            if constexpr (Debug) std::cout << "  Checking rule 2 with activeNode " << activeNode << ", activeEdge " << activeEdge << " and activeLength " << activeLength << std::endl;
            if (activeEdge == NoEdge) return false;
            Node<CharType>* targetEdge = activeNode->getChild(activeEdge);
            if (activeLength < targetEdge->getSubstringLength()) {
                if constexpr (Debug) std::cout << "   inside of an edge" << std::endl;
                //We are inside the current edge, simply see if it continues with the right character.
                if (text[targetEdge->getTextIndex(activeLength)] == newCharacter) {
                    //new character is already there, rule 2 does not apply. We need rule 3.
                    if constexpr (Debug) std::cout << "    new character already there, not rule 2" << std::endl;
                    return false;
                } else {
                    //We need a split inside this edge.
                    if constexpr (Debug) std::cout << "    new character not found, use rule 2" << std::endl;
                    return true;
                }
            } else {
                //We are at the end of an edge. We need rule 2 if there is already another edge but not one for the new character.
                if constexpr (Debug) std::cout << "   at the end of an edge." << std::endl;
                return targetEdge->hasChildren() && !targetEdge->getChild(newCharacter);
            }
        }

        inline std::tuple<Node<CharType>*, CharType, int> walkDown(Node<CharType>* startNode, int gammaStart, int gammaEnd) {
            AssertMsg(startNode != NULL, "Attempted to walk down from NULL.");
            //Skip gammaStart forwards until we reached the end.
            while (true) {
                int g = gammaEnd - gammaStart;//remaining length of string gamma
                Node<CharType>* relevantChild = startNode->getChild(text[gammaStart]);
                AssertMsg(relevantChild != NULL, "Child does not exist in walk down.");
                int gPrime = relevantChild->getSubstringLength();//length of the next edge that must be used.
                if (gPrime < g) {
                    //The end is not in the current edge, skip the edge entirely.
                    startNode = relevantChild;
                    gammaStart += gPrime;//We skipped gPrime characters, mark that accordingly.
                } else {
                    //We found the relevant edge, return it.
                    return std::make_tuple(startNode, text[gammaStart], std::max(0, gammaEnd - gammaStart - 1));
                }
            }
        }

        inline void print() {
            std::cout << std::endl;
            std::cout << "    Printing suffix tree: " << std::endl;
            root.print(4);
            std::cout << std::endl;
        }

    private:
        const   CharType* text;
        Node<CharType> root;
        int currentEnd;
        int n;//input length
        bool usedRule3;
        Node<CharType>* previouslyInsertedNode;
        Node<CharType>* previousExtensionNode;
        int previousExtensionEndIndex;
    };

}
