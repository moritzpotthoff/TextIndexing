#pragma once

#include <iostream>
#include <bits/stdc++.h>
#include <queue>

#include "../NaiveSuffixTree/SuffixTree.h"
#include "../NaiveSuffixTree/Node.h"

namespace Query {
    template<typename CHAR_TYPE, typename PROFILER, bool DEBUG = false>
    class RepeatQuery {
        using CharType = CHAR_TYPE;
        using Profiler = PROFILER;
        static const bool Debug = DEBUG;

    public:
        RepeatQuery(NaiveSuffixTree::SuffixTree<CharType, Debug>* tree) :
            tree(tree),
            suffixes(tree->n) {
            //precompute number of leaves under each node.
            profiler.startStringDepth();
            calculateStringDepths();
            profiler.endStringDepth();
            profiler.startCollectSuffixes();
            collectSuffixesInTextOrder();
            profiler.endCollectSuffixes();
            if constexpr (Debug) {
                std::cout << std::endl << std::endl << std::endl << "Done with query preprocessing. Tree is:" << std::endl;
                tree->root.print(4);
                std::cout << std::endl << std::endl << std::endl << "Suffix pointer array is:" << std::endl;
                for (size_t i = 0; i < tree->n; i++) {
                    std::cout << "   " << i << ": " << suffixes[i] << std::endl;
                }
            }
        }

        inline std::pair<size_t, size_t> runQuery() noexcept {
            profiler.startActualQuery();
            for (int l = tree->n / 2; l >= 0; l--) {
                profiler.startLengthPhase();
                if constexpr (Debug) std::cout << "Checking for length " << l << std::endl;
                for (size_t suffix = 0; suffix < tree->n - (2 * l) + 1; suffix++) {
                    //get lowest common ancestor of suffix nodes for suffix and suffix + l
                    profiler.startLcaPhase();
                    NaiveSuffixTree::Node<CharType>* lcaNode = getLcaNode(suffix, suffix + l);
                    profiler.endLcaPhase();
                    if constexpr (Debug) std::cout << "  Checking string depth for lca node " << lcaNode << std::endl;
                    if (lcaNode->stringDepth == l) {
                        //this is the first solution, return it.
                        profiler.endActualQuery();
                        return std::make_pair(suffix, l);
                    }
                }
                profiler.endLengthPhase();
            }
            profiler.endActualQuery();
            return std::make_pair(0, 0);
        }

        inline NaiveSuffixTree::Node<CharType>* getLcaNode(size_t firstSuffix, size_t secondSuffix) {
            if constexpr (Debug) std::cout << "  Computing LCA for " << firstSuffix << ", " << secondSuffix << std::endl;
            NaiveSuffixTree::Node<CharType>* firstNode = suffixes[firstSuffix];
            NaiveSuffixTree::Node<CharType>* secondNode = suffixes[secondSuffix];
            //get paths from both nodes to the root
            std::vector<NaiveSuffixTree::Node<CharType>*> firstPath;
            std::vector<NaiveSuffixTree::Node<CharType>*> secondPath;
            //TODO reserve size.
            while (firstNode != &tree->root) {
                firstPath.push_back(firstNode);
                firstNode = firstNode->parent;
            }
            while (secondNode != &tree->root) {
                secondPath.push_back(secondNode);
                secondNode = secondNode->parent;
            }
            firstPath.push_back(&tree->root);
            secondPath.push_back(&tree->root);
            if constexpr (Debug) std::cout << "    Got paths" << std::endl;
            //find last equal node from the back of the two vertices (that is the lca)
            size_t firstElement = firstPath.size() - 1;
            size_t secondElement = secondPath.size() - 1;
            while (firstPath[firstElement] == secondPath[secondElement]) {
                firstElement--;
                secondElement--;
            }
            if constexpr (Debug) std::cout << "    Found lca" << std::endl;
            return firstPath[firstElement + 1];
        }

        inline void collectSuffixesInTextOrder() {
            std::queue<NaiveSuffixTree::Node<CharType>*> queue;
            queue.push(&tree->root);
            while (!queue.empty()) {
                NaiveSuffixTree::Node<CharType>* node = queue.front();
                queue.pop();
                if (node->hasChildren()) {
                    //not a leaf, explore the children instead
                    for (const auto & [key, child] : node->children) {
                        queue.push(child);
                    }
                } else {
                    //leaf, representing a suffix, add to the list.
                    suffixes[node->representedSuffix] = node;
                }
            }
        }

        inline void calculateStringDepths() noexcept {
            stringDepthDfs(&tree->root, 0);
        }

        inline void stringDepthDfs(NaiveSuffixTree::Node<CharType>* node, size_t depth) noexcept {
            //TODO avoid recursion?
            node->stringDepth = depth + node->endIndex - node->startIndex;
            for (const auto & [key, child] : node->children) {
                stringDepthDfs(child, node->stringDepth);
            }
        }

    public:
        NaiveSuffixTree::SuffixTree<CharType, Debug>* tree;
        std::vector<NaiveSuffixTree::Node<CharType>*> suffixes;

        Profiler profiler;
    };
}
