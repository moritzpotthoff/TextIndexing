#pragma once

#include <map>
#include "Helpers.h"

namespace SuffixTree {
    template<typename CHAR_TYPE>
    class Node {
        using CharType = CHAR_TYPE;

    public:
        Node(Node<CharType>* parent = NULL, int startIndex = 0, int endIndex = 0, Node<CharType>* suffixLink = NULL) :
                parent(parent),
                startIndex(startIndex),
                endIndex(endIndex),
                suffixLink(suffixLink) {
        }

        inline int getActivePointIndex(CharType activeEdge, int activeLength) {
            Node<CharType>* targetChild = children[activeEdge];
            return targetChild->getTextIndex(activeLength);
        }

        inline int getTextIndex(int offset) {
            AssertMsg((offset >= 0 && startIndex + offset < endIndex), "Offset is invalid.");
            return startIndex + offset;
        }

        inline Node<CharType>* splitEdge(CharType activeEdge, int activeLength, CharType previousActivePointCharacter, CharType newCharacter) {
            Node<CharType>* splitterNode = new Node(this, startIndex, startIndex + activeLength, NULL);
            splitterNode->addChild(previousActivePointCharacter, children[activeEdge]);
            Node<CharType>* leafNode = new Node(splitterNode, startIndex + activeLength, startIndex + activeLength + 1, NULL);
            splitterNode->addChild(newCharacter, leafNode);
            children[activeEdge] = splitterNode;
            return splitterNode;
        }

        inline void addChild(CharType key, Node* value) {
            value->setParent(this);
            children[key] = value;
        }

        inline Node<CharType>* getChild(CharType key) {
            return children[key];
        }

        inline void setParent(Node* newParent) {
            parent = newParent;
        }

        inline Node<CharType>* getSuffixLink() {
            return suffixLink;
        }

        inline void addSuffixLink(Node* newSuffixLink) {
            suffixLink = newSuffixLink;
        }

        inline int getEndIndex() {
            return endIndex;
        }

    private:
        Node<CharType>* parent;
        int startIndex;//inclusive
        int endIndex;//exclusive
        Node<CharType>* suffixLink;
        std::map<CharType, Node<CharType>*> children;
    };

}

