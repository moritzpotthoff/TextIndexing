#pragma once

#include <map>
#include "Helpers.h"

namespace UkkonenSuffixTree {
    template<typename CHAR_TYPE>
    class Node {
        using CharType = CHAR_TYPE;
        static const CharType NoEdge = 127;//TODO fix

    public:
        Node(Node<CharType>* parent, int startIndex, int endIndex, Node<CharType>* suffixLink) :
                parent(parent),
                startIndex(startIndex),
                endIndex(endIndex),
                suffixLink(suffixLink) {
        }

        inline int getActivePointIndex(CharType activeEdge, int activeLength) {
            if (activeEdge == NoEdge) return activeLength;
            AssertMsg(children[activeEdge] != NULL, "Child does not exist.");
            return children[activeEdge]->getTextIndex(activeLength);
        }

        inline int getTextIndex(int offset) {
            AssertMsg((offset >= 0 && startIndex + offset < endIndex), "Offset is invalid.");
            return startIndex + offset;
        }

        inline Node<CharType>* splitEdge(CharType activeEdge, int activeLength, CharType splitCharacter) {
            //Add new splitter node that has this nodes parent and the appropriate edge section.
            Node<CharType>* splitterNode = new Node(parent, startIndex, startIndex + activeLength, NULL);
            //Adjust the incoming edge of this node accordingly. EndIndex remains unchanged.
            parent = splitterNode;
            startIndex = startIndex + activeLength;
            parent->addChild(activeEdge, splitterNode);//adjust the incoming edge from parent to splitterNode.
            splitterNode->addChild(splitCharacter, this);//add the edge from splitterNode to this.
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

        inline int getStartIndex() {
            return startIndex;
        }

        inline int getSubstringLength() {
            return endIndex - startIndex;
        }

        inline Node<CharType>* walkUp(int currentIndex, Node<CharType>* root, int* substringStartIndex, int* substringEndIndex) {
            if (this == root || suffixLink != NULL) {
                //this node is the one we want, no walk-up necessary. The used string is empty.
                *substringStartIndex = endIndex;
                *substringEndIndex = endIndex;
                return this;
            } else {
                //walk up the edge to the parent pointer. The used string is that along the edge pointing to this.
                *substringStartIndex = startIndex;
                *substringEndIndex = currentIndex;
                AssertMsg(parent != NULL, "Found null node on walkUp.");
                return parent;
            }
        }

        inline bool hasChildren() {
            return !children.empty();
        }

        inline void print(int depth) {
            std::cout << std::string(depth, ' ') << "Node " << this << " with startIndex " << startIndex << ", endIndex " << endIndex << ", suffixLink " << suffixLink << " and parent " << parent << std::endl;
            for (std::pair<CharType, Node<CharType>*> element : children) {
                std::cout << std::string(depth + 2, ' ') << "Key " << element.first << " is child " << element.second << std::endl;
                element.second->print(depth + 4);
            }
        }

    private:
        Node<CharType>* parent;
        int startIndex;//inclusive
        int endIndex;//exclusive
        Node<CharType>* suffixLink;
        std::map<CharType, Node<CharType>*> children;
    };

}

