#pragma once

#include <iostream>
#include <bits/stdc++.h>
#include <queue>

#include "../NaiveSuffixTree/SuffixTree.h"
#include "../NaiveSuffixTree/Node.h"

namespace Query {
    struct Candidate {
        size_t occurences; //number of occurences of the substring
        size_t startPosition; //start index in the text
    };

    template<typename CHAR_TYPE, typename PROFILER, bool DEBUG = false>
    class TopKQuery {
        using CharType = CHAR_TYPE;
        using Profiler = PROFILER;
        static const bool Debug = DEBUG;

    public:
        TopKQuery(NaiveSuffixTree::SuffixTree<CharType, Debug>* tree) :
            tree(tree) {
            profiler.startInitialization();
            //precompute number of leaves under each node.
            countNumberOfLeaves();
            profiler.endInitialization();
            if constexpr (Debug) tree->root.print(4);
        }

        inline size_t runQuery(size_t l, size_t k) noexcept {
            profiler.startNewQuery();

            if constexpr (Debug) std::cout << "Running topk query with l = " << l << " and k = " << k << std::endl;
            //collect all relevant pairs of #occurences and suffix start
            profiler.startCollectCandidates();
            std::vector<Candidate> candidates;
            candidates.reserve(tree->n / 10);//rough estimate. This worked well in my evaluations.
            collectingBfs(candidates, l);
            candidates.shrink_to_fit();//speeds up the later stable sort: now we know the size, so require only that capacity so that memory allocations during stable_sort are faster.
            profiler.endCollectCandidates();

            //now, stable-sort the candidates by #occurences only to find the lexicographically smallest one of all with
            //the same #occurences (they were added in lexicographic order due to suffix tree order)
            profiler.startSortCandidates();
            std::stable_sort(candidates.begin(), candidates.end(), [](const Candidate& left, const Candidate& right){
                return left.occurences > right.occurences;
            });
            profiler.endSortCandidates();
            if constexpr (Debug) {
                std::cout << "Found sorted candidates: " << std::endl;
                for (auto &candidate : candidates) {
                    std::cout << "Start index: " << candidate.startPosition << ", #occ. = " << candidate.occurences << std::endl;
                }
            }

            profiler.startReconstructSolution();
            //finally, select the k-th element
            Candidate& solution = candidates[k - 1];

            //reconstruct the solution
            size_t startIndex = solution.startPosition;
            size_t endIndex = startIndex + l;//inclusively
            profiler.endReconstructSolution();
            if constexpr (Debug) std::cout << "Found suffix from " << startIndex << " to " << endIndex << std::endl;
            profiler.endCurrentQuery();
            return startIndex;
        }

        inline void collectingBfs(std::vector<Candidate>& candidates, const size_t length) const noexcept {
            std::queue<NaiveSuffixTree::Node<CharType>*> queue;
            queue.push(&tree->root);
            while (!queue.empty()) {
                const NaiveSuffixTree::Node<CharType>* node = queue.front();
                queue.pop();
                //If this node has at least level l (and is the highest one along its path to do so), add it as candidate
                if (node->stringDepth >= length) {
                    //add this as a candidate. To reconstruct the pattern, store the start index of the suffix
                    candidates.emplace_back(node->numberOfLeaves, node->representedSuffix);
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
            node->stringDepth = depth + node->endIndex - node->startIndex;
            if (node->hasChildren()) {
                //calculate represented suffix also for non leaves.
                node->representedSuffix = node->endIndex - node->stringDepth;
                //inner node, sum up over all children. node->numberOfLeaves is already initialized to 0.
                for (const auto & [key, child] : node->children) {
                    node->numberOfLeaves += countingDfs(child, node->stringDepth);
                }
                return node->numberOfLeaves;
            } else {
                //this is a leaf
                node->numberOfLeaves = 1;
                return 1;
            }
        }

    public:
        NaiveSuffixTree::SuffixTree<CharType, Debug>* tree;

        Profiler profiler;
    };
}
