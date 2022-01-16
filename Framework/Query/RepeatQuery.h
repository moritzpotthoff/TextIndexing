#pragma once

#include <iostream>
#include <bits/stdc++.h>
#include <queue>
#include <algorithm>

#include "../NaiveSuffixTree/SuffixTree.h"
#include "../NaiveSuffixTree/Node.h"

namespace Query {
    /**
     * Brief summary of the thoughts that led to my query approach:
     *  - If there is a substring aa, then the text must have the form uaav.
     *  - This means that there are two suffixes, aav and av starting with the same prefix a of length l.
     *    The suffixes have indices i and i+l for some i.
     *  - Consequently, for any substring of the form aa with length 2*l, there exists an inner node in the suffix tree
     *    with suffix depth l (the path to it represents the common prefix, a), that has two leaves below it which
     *    represent exactly the suffixes i and i+l and vice-versa.
     *  - Find such a node and we get the result.
     *
     * Therefore, the query works as follows:
     *  - Precompute suffix depths in the suffix tree.
     *  - Collect all inner nodes, stable-sorted by their suffix depth (descending); stable-sort to preserve lexicographic ordering.
     *  - For each inner node n with suffix depth l:
     *      - Get the sorted suffix indices for all suffixes represented by leaves below n
     *        Observe that all inner nodes below n were already considered (they have greater suffix depth).
     *        Therefore, we can obtain the sorted list of suffixes
     *        by *merging* the (available) lists of suffixes for each child that is an inner node
     *        and inserting suffixes for all children that are already leaves.
     *      - In the sorted list of suffixes, efficiently check if there is any pair of indices with difference of exactly l.
     *        By the above observation, such a pair is a witness for a substring aa starting at the smaller of the two suffixes that
     *        make up the pair.
     *  - Because we consider inner nodes by descending string depth, the first witness is the result.
     *  - Return the suffix start position.
     */
    template<typename CHAR_TYPE, typename PROFILER, bool DEBUG = false>
    class RepeatQuery {
        using CharType = CHAR_TYPE;
        using Profiler = PROFILER;
        static const bool Debug = DEBUG;

    public:
        RepeatQuery(NaiveSuffixTree::SuffixTree<CharType, Debug>* tree) :
            tree(tree) {
            //precompute number of leaves under each node.
            profiler.startStringDepth();
            calculateStringDepths();
            profiler.endStringDepth();
            profiler.startCollectInnerNodes();
            collectInnerNodes();
            profiler.endCollectInnerNodes();
            if constexpr (Debug) {
                std::cout << std::endl << std::endl << std::endl << "Done with query preprocessing. Tree is:" << std::endl;
                tree->root.print(4);
            }
        }

        inline std::pair<size_t, size_t> runQuery() noexcept {
            profiler.startActualQuery();
            for (NaiveSuffixTree::Node<CharType>* innerNode : sortedInnerNodes) {
                profiler.startInnerNodePhase();
                if constexpr (Debug) std::cout << "Looking at inner node with depth " << innerNode->stringDepth << std::endl;
                collectSuffixesBelow(innerNode);
                std::vector<size_t> leaves = suffixesBelowInnerNode[innerNode->representedSuffix];
                if constexpr (Debug) {
                    std::cout << "   Leaves below it are: " << innerNode->stringDepth << std::endl << "      ";
                    for (size_t i = 0; i < leaves.size(); i++) {
                        std::cout << leaves[i] << ", ";
                    }
                    std::cout << std::endl;
                }
                //leaves are already sorted
                bool foundSolution;
                size_t startIndex;
                profiler.startPairPhase();
                std::tie(foundSolution, startIndex) = findPair(leaves, innerNode->stringDepth);
                profiler.endPairPhase();
                if (foundSolution) {
                    if constexpr (Debug) std::cout << "   Found solution at position " << startIndex << std::endl;
                    profiler.endInnerNodePhase();
                    profiler.endActualQuery();
                    return std::make_pair(startIndex, 2 * innerNode->stringDepth);
                }
                if constexpr (Debug) std::cout << "   Found no solution." << std::endl;
                profiler.endInnerNodePhase();
            }
            profiler.endActualQuery();
            return std::make_pair(0, 0);
        }

        inline std::pair<bool, size_t> findPair(std::vector<size_t> &leaves, size_t difference) const noexcept {
            // https://www.geeksforgeeks.org/find-a-pair-with-the-given-difference/
            size_t i = 0;
            size_t j = 1;

            while (i < leaves.size() && j < leaves.size()) {
                if (i != j && (leaves[std::max(i, j)] - leaves[std::min(i, j)] == difference )) {
                    //found a pair, return first index
                    return std::make_pair(true, leaves[std::min(i, j)]);
                } else if (leaves[j] - leaves[i] < difference) {
                    j++;
                } else {
                    i++;
                }
            }
            return std::make_pair(false, 0);
        }

        inline void collectSuffixesBelow(NaiveSuffixTree::Node<CharType>* innerNode) noexcept {
            std::vector<size_t> suffixes;
            const size_t currentIndex = innerNode->representedSuffix;
            for (const auto & [key, child] : innerNode->children) {
                if (child->hasChildren()) {
                    //child is inner node, reuse previously generated list
                    const size_t childIndex = child->representedSuffix;
                    suffixes.clear();
                    std::merge(suffixesBelowInnerNode[currentIndex].begin(), suffixesBelowInnerNode[currentIndex].end(), suffixesBelowInnerNode[childIndex].begin(), suffixesBelowInnerNode[childIndex].end(), std::back_inserter(suffixes));
                    //copy new list back
                    suffixesBelowInnerNode[currentIndex].clear();
                    std::copy(suffixes.begin(), suffixes.end(), std::back_inserter(suffixesBelowInnerNode[currentIndex]));
                } else {
                    //child is leaf, insert its suffix only.
                    suffixesBelowInnerNode[currentIndex].insert(std::upper_bound(suffixesBelowInnerNode[currentIndex].begin(), suffixesBelowInnerNode[currentIndex].end(), child->representedSuffix), child->representedSuffix);
                }
            }
        }

        inline void collectInnerNodes() noexcept {
            sortedInnerNodes.reserve(tree->n);
            std::queue<NaiveSuffixTree::Node<CharType>*> queue;
            queue.push(&tree->root);
            while (!queue.empty()) {
                NaiveSuffixTree::Node<CharType>* node = queue.front();
                queue.pop();
                if (node->hasChildren()) {
                    //inner node, enter as inner node
                    sortedInnerNodes.emplace_back(node);
                    for (const auto & [key, child] : node->children) {
                        queue.push(child);
                    }
                }
                //nothing to do for leaves
            }
            //stable-sort by suffix depths
            sortedInnerNodes.shrink_to_fit();
            std::stable_sort(sortedInnerNodes.begin(), sortedInnerNodes.end(), [](const NaiveSuffixTree::Node<CharType>* left, const NaiveSuffixTree::Node<CharType>* right){
                return left->stringDepth > right->stringDepth;
            });
            for (size_t i = 0; i < sortedInnerNodes.size(); i++) {
                sortedInnerNodes[i]->representedSuffix = i;
            }
            suffixesBelowInnerNode.resize(sortedInnerNodes.size());
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
        std::vector<NaiveSuffixTree::Node<CharType>*> sortedInnerNodes;
        std::vector<std::vector<size_t>> suffixesBelowInnerNode;

        Profiler profiler;
    };
}
