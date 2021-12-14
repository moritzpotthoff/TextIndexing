#pragma once

#include <iostream>
#include <bits/stdc++.h>

#include "Node.h"

namespace SuffixTree {

    template<typename CHAR_TYPE>
    class SuffixTree {
        using CharType = CHAR_TYPE;
        static const int CurrentEndFlag = INT_MAX;
        static const CharType NoEdge = 255;//TODO fix

    public:
        SuffixTree(const CharType* input, int n) :
            text(input),
            root(NULL, 0, CurrentEndFlag, NULL),
            currentEnd(0),
            activeNode(&root),
            activeEdge(NoEdge),
            activeLength(0),
            remainder(1),
            n(n) {
            std::cout << "Created new suffix tree." << std::endl;

            //iterate over the input, for each new character, add the necessary suffixes in the new phase
            for (int i = 0; i < n; i++) {
                singlePhase(i);
            }
        }

        inline int singlePhase(int phase) {
            //Update currentEnd index to automatically extend all existing edges. This handles all implicit extensions.
            currentEnd = phase;

            int previousEnd = 0;
            for (int j = 0; j < phase + 1; j++) {
                if (singleExtension(j)) {
                    //Rule 3 was applied, this phase ends.
                    previousEnd = j - 1;
                    break;
                }
                previousEnd = j;
            }

            return 0;
        }

        inline bool singleExtension(int extensionEndIndex) {
            /* By design, the path for the current suffix ends at the active-Triple.
             * Go there and see if anything needs to be added.
             **/
            AssertMsg(extensionEndIndex < n, "Accessed beyond text limit.");
            CharType newCharacter = text[extensionEndIndex];
            int activePointIndex = activeNode->getActivePointIndex(activeEdge, activeLength);
            AssertMsg(activePointIndex < n, "Accessed beyond text limit.");
            CharType activePointCharacter = text[activePointIndex];

            if (activePointCharacter == newCharacter) {
                //Suffix is implicitly represented by the active edge, nothing to do.
                remainder++;
                activeLength++;
                if (activeLength == activeNode->getEndIndex()) {
                    //Update active node because we currently point behind it.
                    activeNode = activeNode->getChild(activeEdge);
                    activeLength = 0;
                    activeEdge = newCharacter;//TODO check that this is the correct new character.
                }
            } else {
                //The new character is not present, we need to split the edge accordingly.
                Node<CharType>* insertedNode = activeNode->splitEdge(activeEdge, activeLength, activePointCharacter, newCharacter);
                if (lastInsertedNode != NULL) {
                    //Rule 2 [reddit]: We need to add the suffix link from the lastInsertedNode to the currently insertedNode.
                    lastInsertedNode->addSuffixLink(insertedNode);
                }
                remainder--;
                //Update the active point.
                if (activeNode == &root) {
                    //Rule 1 [reddit].
                    //activeNode remains unchanged
                    activeLength--;
                    //activeEdge = ??;//TODO
                } else {
                    //Rule 3 [reddit]
                    activeNode = activeNode->getSuffixLink();
                    if (activeNode == NULL) activeNode = &root;
                    //rest remains unchanged.
                    return true;
                }
            }
            return false;
        }

    private:
        const   CharType* text;
        Node<CharType> root;
        int currentEnd;
        Node<CharType>* activeNode;
        CharType activeEdge;
        int activeLength;
        int remainder;
        int n;//input length
        Node<CharType>* lastInsertedNode;
    };

}
