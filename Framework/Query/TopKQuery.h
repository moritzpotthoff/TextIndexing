#pragma once

#include <iostream>
#include <bits/stdc++.h>
#include <queue>

#include "../NaiveSuffixTree/SuffixTree.h"
#include "../NaiveSuffixTree/Node.h"

namespace Query {
    template<typename CHAR_TYPE, bool DEBUG = false>
    class TopKQuery {
        using CharType = CHAR_TYPE;
        static const bool Debug = DEBUG;

    public:
        TopKQuery(NaiveSuffixTree::SuffixTree<CharType, Debug>* tree) :
            tree(tree) {
            //precompute number of leaves under each node.
            countNumberOfLeaves();
            if constexpr (Debug) tree->root.print(4);
        }

        inline size_t runQuery(size_t newL, size_t newK) noexcept {
            l = newL;
            k = newK;
            if constexpr (Debug) std::cout << "Running topk query with l = " << l << " and k = " << k << std::endl;
            //collect all relevant pairs of #occurences and suffix start
            std::vector<std::pair<size_t, size_t>> candidates;
            candidates.reserve(tree->n / 2);
            collectingBfs(candidates);

            //now, stable-sort the candidates to find the lexicographically smallest one (they were added in lexicographic order)
            std::stable_sort(candidates.begin(), candidates.end(), [](const std::pair<size_t, size_t>& left, const std::pair<size_t, size_t>& right){
                return left.first > right.first;
            });
            if constexpr (Debug) {
                std::cout << "Found sorted candidates: " << std::endl;
                for (auto &candidate : candidates) {
                    std::cout << "Start index: " << candidate.second << ", #occ. = " << candidate.first << std::endl;
                }
            }
            //finally, select the k-th element
            std::pair<size_t, size_t>& solution = candidates[k - 1];
            //reconstruct the solution
            size_t startIndex = solution.second;
            size_t endIndex = startIndex + l;//inclusively
            if constexpr (Debug) std::cout << "Found suffix from " << startIndex << " to " << endIndex << std::endl;
            return startIndex;
        }

        inline void collectingBfs(std::vector<std::pair<size_t, size_t>>& candidates) {
            std::queue<NaiveSuffixTree::Node<CharType>*> queue;
            queue.push(&tree->root);
            while (!queue.empty()) {
                NaiveSuffixTree::Node<CharType>* node = queue.front();
                queue.pop();
                //If this node has at least level l (and is the highest one along its path to do so), add it as candidate
                if (node->previousStringDepth + node->endIndex - node->startIndex >= l) {
                    //add this as a candidate. To reconstruct the pattern, store the start index of the suffix
                    candidates.push_back(std::make_pair<size_t, size_t>(size_t(node->numberOfLeaves), node->startIndex - node->previousStringDepth));
                } else {
                    //Explore children, except if the node has been added. Then, children would be redundant.
                    for (const auto & [key, child] : node->children) {
                        queue.push(child);
                    }
                }
            }
        }

        inline void countNumberOfLeaves() noexcept {
            countingDfs(&tree->root, 0);
        }

        inline size_t countingDfs(NaiveSuffixTree::Node<CharType>* node, size_t depth) noexcept {
            //TODO avoid recursion?
            node->previousStringDepth = depth;
            if (node->hasChildren()) {
                //inner node, sum up over all children
                node->numberOfLeaves = 0;
                for (const auto & [key, child] : node->children) {
                    node->numberOfLeaves += countingDfs(child, depth + node->endIndex - node->startIndex);
                }
                return node->numberOfLeaves;
            } else {
                //this is a leaf
                node->numberOfLeaves = 1;
                return 1;
            }
        }

    private:
        NaiveSuffixTree::SuffixTree<CharType, Debug>* tree;
        size_t l;
        size_t k;
    };
}
