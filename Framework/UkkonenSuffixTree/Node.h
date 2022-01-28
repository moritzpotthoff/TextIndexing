#pragma once

#include <map>
#include "Helpers.h"

namespace UkkonenSuffixTree {
    template<typename CHAR_TYPE>
    class Node {
        using CharType = CHAR_TYPE;
        static const CharType NoEdge = 127;//TODO fix
        static const int CurrentEndFlag = -1; //marks that an edges end is actually the global currentEnd

    public:
        Node(Node<CharType>* parent, int startIndex, int endIndex, Node<CharType>* suffixLink = NULL) :
                parent(parent),
                startIndex(startIndex),
                endIndex(endIndex),
                suffixLink(suffixLink) {
            AssertMsg(endIndex == 0 || endIndex == CurrentEndFlag || endIndex > startIndex, "Wrong node indices");
            if (endIndex <= startIndex && endIndex != CurrentEndFlag) {
                std::cout << "WRONG NODE INDICES [" << startIndex << ", " << endIndex << "), parent " << parent << std::endl;
            }
        }

        inline int getActivePointIndex(CharType activeEdge, int activeLength, int currentEnd) noexcept {
            if (activeEdge == NoEdge) return activeLength;
            return children[activeEdge]->getTextIndex(activeLength, currentEnd);
        }

        inline int getTextIndex(int offset, int currentEnd) const noexcept {
            int actualEndIndex = endIndex;
            if (endIndex == CurrentEndFlag) actualEndIndex = currentEnd;
            AssertMsg((offset >= 0 && startIndex + offset < actualEndIndex), "Offset is invalid.");
            return startIndex + offset;
        }

        inline int trueEndIndex(int currentEnd) const noexcept {
            if (endIndex == CurrentEndFlag) return currentEnd;
            return endIndex;
        }

        inline void addChild(CharType key, Node* value) noexcept {
            value->parent = this;
            children[key] = value;
        }

        inline Node<CharType>* getChild(CharType key) const noexcept {
            auto child = children.find(key);
            if (child == children.end()) return NULL;
            //std::cout << "xxxxx found child: " << child->first << ", " << child->second << std::endl;
            return child->second;
        }

        inline int getSubstringLength(int currentEnd) const noexcept {
            if (endIndex == CurrentEndFlag) { //ukkonen trick
                return currentEnd - startIndex;
            }
            return endIndex - startIndex;
        }

        inline bool hasChildren() const noexcept {
            return !children.empty();
        }

        inline void print(int depth) const noexcept {
            std::cout << std::string(depth, ' ') << "Node " << this << " [" << startIndex << ", " << endIndex << "), suffixLink " << suffixLink << " and parent " << parent << std::endl;
            for (std::pair<CharType, Node<CharType>*> element : children) {
                std::cout << std::string(depth + 2, ' ') << "Key " << element.first << " is child " << element.second << std::endl;
                element.second->print(depth + 4);
            }
        }

    public:
        Node<CharType>* parent;
        int startIndex;//inclusive
        int endIndex;//exclusive
        Node<CharType>* suffixLink;
        std::map<CharType, Node<CharType>*> children;
    };

}

