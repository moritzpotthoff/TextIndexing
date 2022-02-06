#pragma once

#include <iostream>
#include <bits/stdc++.h>
#include <queue>
#include <algorithm>

#include "../UkkonenSuffixTree/SuffixTree.h"
#include "../UkkonenSuffixTree/Node.h"

namespace Query {
    /**
     * Brief summary of the thoughts that led to my query approach:
     *  - If there is a substring aa, then the text must have the form uaav.
     *  - This means that there are two suffixes, aav and av, starting with the same prefix a of length l.
     *    These suffixes have indices i and i+l for some i.
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
    template<typename CHAR_TYPE, CHAR_TYPE SENTINEL, typename PROFILER, bool DEBUG = false>
    class RepeatQuery {
        using CharType = CHAR_TYPE;
        using Profiler = PROFILER;
        static const CharType Sentinel = SENTINEL;
        static const bool Debug = DEBUG;

    public:
        /**
         * Create a new query from the given suffix tree tree.
         * Does some necessary preprocessing steps that would typically be made in the suffix tree
         * generation if my suffix tree would be used only for this query type.
         * Therefore, they are here and counted as preprocessing time.
         */
        RepeatQuery(SuffixTree::SuffixTree<CharType, Debug>* tree) :
            tree(tree) {
            //precompute number of leaves under each node.
            profiler.startStringDepth();
            calculateStringDepths();
            profiler.endStringDepth();
            //collect all inner nodes in sorted order
            profiler.startCollectInnerNodes();
            collectInnerNodes();
            profiler.endCollectInnerNodes();
            if constexpr (Debug) {
                std::cout << std::endl << std::endl << std::endl << "Done with query preprocessing. Tree is:" << std::endl;
                tree->root.print(4);
                for (SuffixTree::Node<CharType>* innerNode : sortedInnerNodes) {
                    std::cout << "d=" << innerNode->stringDepth << ",c=" << tree->text[innerNode->startIndex] << std::endl;
                }
                std::cout << std::endl;
            }
        }

        /**
         * Actually runs the repeat query on the input tree using the precomputed values and the
         * algorithm described above.
         *
         * Returns the start index and the length of the repetition (length of aa).
         */
        inline std::pair<size_t, size_t> runQuery() noexcept {
            profiler.startActualQuery();
            //iterate over all inner nodes, they are already in sorted order.
            for (SuffixTree::Node<CharType>* innerNode : sortedInnerNodes) {
                profiler.startInnerNodePhase();
                //get all the suffixes below the inner node using the DP-merging approach described above
                profiler.startMergePhase();
                collectSuffixesBelow(innerNode);
                std::vector<size_t> leaves = suffixesBelowInnerNode[innerNode->representedSuffix];
                profiler.endMergePhase();
                profiler.startPairPhase();
                //find a pair of suffix indices below innerNode whose difference is the innerNode's string depth.
                int startIndex = findPair(leaves, innerNode->stringDepth);
                profiler.endPairPhase();
                if (startIndex != -1) {
                    profiler.endInnerNodePhase();
                    profiler.endActualQuery();
                    //return solution; since we consider inner nodes by descending string depth, the first inner node with an
                    //appropriate pair of suffixes below it is the solution; return the start index and the length.
                    return std::make_pair(startIndex, 2 * innerNode->stringDepth);
                }
                profiler.endInnerNodePhase();
            }
            profiler.endActualQuery();
            return std::make_pair(0, 0);
        }

        /**
         * In the sorted list of suffix indices leaves, finds a pair of values with the given difference, if it exists.
         *
         * Returns if a result was found and the lexicographically smaller suffix index
         */
        inline int findPair(std::vector<size_t> &leaves, size_t difference) const noexcept {
            /**
             * Instead of the simple O(n log n)-approach (for each element, binary-search for the counterpart),
             * I use this O(n) algorithm that I found at
             *      https://www.geeksforgeeks.org/find-a-pair-with-the-given-difference/ (last accessed: 01/31/2022)
             *
             */
            //two pointers into the vector
            size_t i = 0;
            size_t j = 1;

            //iterate over the input, as long as no pointer has reached the end
            while (i < leaves.size() && j < leaves.size()) {
                //if we have a pair with the desired difference, return it. If there are other possibilities, those are irrelevant.
                if (i != j && (leaves[i] - leaves[j] == difference || leaves[j] - leaves[i] == difference)) {
                    //found a pair, return first index
                    return leaves[std::min(i, j)];
                } else if (leaves[j] - leaves[i] < difference) { //the difference of the two values is smaller than the desired value, we need to increase the right value, therefore increase j
                    j++;
                } else { //increase i
                    i++;
                }
            }
            //no result found, return dummy instead.
            return -1;
        }

        /**
         * Over the course of the query, this is a dynamic program that computes all necessary sorted list of
         * suffix indices below inner nodes.
         *
         * For a call for innerNode, it uses the lists for all children of innerNode.
         * Again, these were already computed and stored since inner nodes with greater string depth were considered before.
         *
         * Therefore, it is sufficient to iteratively merge all those lists.
         * This could probably be made more efficient using k-way merging, but I don't have that much time for now.
         *
         * The resulting list will be stored into suffixesBelowInnerNode[innerNode->representedSuffix]
         */
        inline void collectSuffixesBelow(SuffixTree::Node<CharType>* innerNode) noexcept {
            std::vector<size_t> suffixes;
            //index of the list in suffixesBelowInnerNode that the list must be stored in
            const size_t currentIndex = innerNode->representedSuffix;
            for (const auto & [key, child] : innerNode->children) {
                if (child->hasChildren()) {
                    //child is inner node, reuse previously generated list and merge with current list
                    const size_t childIndex = child->representedSuffix;
                    suffixes.clear();
                    //merge the two lists into suffixes
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

        /**
         * Collects all inner nodes in the tree sorted by string depth and lexicographically.
         */
        inline void collectInnerNodes() noexcept {
            //reserve sufficient space to avoid reallocation
            sortedInnerNodes.reserve(tree->n);
            //start bfs in the tree (needed to preserve lexicographic order)
            std::queue<SuffixTree::Node<CharType>*> queue;
            queue.push(&tree->root);
            while (!queue.empty()) {
                SuffixTree::Node<CharType>* node = queue.front();
                queue.pop();
                if (node->hasChildren()) {
                    //inner node, enter as inner node
                    sortedInnerNodes.emplace_back(node);
                    //enqueue the children
                    for (const auto & [key, child] : node->children) {
                        queue.push(child);
                    }
                }
                //nothing to do for leaves
            }
            //reduce container size to save some time during stable-sort
            sortedInnerNodes.shrink_to_fit();
            //stable-sort by suffix depths (descending), stable-sort to preserve lexicographic ordering
            std::stable_sort(sortedInnerNodes.begin(), sortedInnerNodes.end(), [](const SuffixTree::Node<CharType>* left, const SuffixTree::Node<CharType>* right){
                return left->stringDepth > right->stringDepth;
            });
            //prepare DP-memory container size
            suffixesBelowInnerNode.resize(sortedInnerNodes.size());
                for (size_t i = 0; i < sortedInnerNodes.size(); i++) {
                    sortedInnerNodes[i]->representedSuffix = i;
                }
        }

        /**
         * Annotates every node with its string depth.
         */
        inline void calculateStringDepths() noexcept {
            stringDepthDfs(&tree->root, 0);
        }

        /**
         * Recursively annotates each node with its string depth using depth first search.
         *
         * Annotates each node with the suffix it represents. That will be used as ID  to access
         * the precomputed list of suffixes below inner nodes during the dynamic program part.
         */
        inline void stringDepthDfs(SuffixTree::Node<CharType>* node, size_t depth) noexcept {
            node->stringDepth = depth + *node->endIndex - node->startIndex;
            node->representedSuffix = *node->endIndex - node->stringDepth;
            for (const auto & [key, child] : node->children) {
                stringDepthDfs(child, node->stringDepth);
            }
        }

    public:
        SuffixTree::SuffixTree<CharType, Debug>* tree;
        //List of all inner nodes, sorted by their string depths
        std::vector<SuffixTree::Node<CharType>*> sortedInnerNodes;
        //Memory-vector for the dynamic program for suffix-collection.
        //If it was computed, the list of suffixes for an innerNode is accessible at suffixesBelowInnerNode[innerNode->representedSuffix]
        std::vector<std::vector<size_t>> suffixesBelowInnerNode;

        Profiler profiler;
    };
}
