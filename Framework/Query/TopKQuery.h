#pragma once

#include <iostream>
#include <bits/stdc++.h>
#include <queue>

#include "../UkkonenSuffixTree/SuffixTree.h"
#include "../UkkonenSuffixTree/Node.h"

namespace Query {
    /**
     * A Candidate represents a substring starting at startPosition that exists occurences times in the input text.
     */
    struct Candidate {
        int occurences;
        int startPosition;
    };

    /**
     * The overall query idea, explained in more detail below:
     *  - Precompute the string depth of all nodes.
     *  - Precompute the number of leaves under each node.
     *    That is equal to the number of suffixes that this node represents, which all have the same prefix of length of its string depth.
     *    Therefore, it is equal to the number of substrings of their stringDepth in the input.
     *  - Select all highest nodes with string depth >= l. These represent substrings of length >= l (if > l, then a prefix of length l occurs exactly as often).
     *  - Stable-sort those candidates by their #occurences to find the k-th entry. Return that.
     */
    template<typename CHAR_TYPE, CHAR_TYPE SENTINEL, typename PROFILER, bool DEBUG = false>
    class TopKQuery {
        using CharType = CHAR_TYPE;
        using Profiler = PROFILER;//For evaluation, use with TopKProfiler; for production, use NoProfiler. All method calls made to profiler in this class are for time measurements.
        static const CharType Sentinel = SENTINEL;
        static const bool Debug = DEBUG;

    public:
        /**
         * Generates a new query and already does some additional preprocessing on the suffix tree that will be needed later.
         */
        TopKQuery(SuffixTree::SuffixTree<CharType, Debug>* tree) :
            tree(tree) {
            profiler.startInitialization();
            //Additional precomputations that are necessary for the topK queries. Needs to be done only once for all queries.
            countNumberOfLeaves();
            profiler.endInitialization();
            if constexpr (Debug) tree->root.print(4);
        }

        /**
         * Runs a query for the given length l and finds the k-th most frequent substring.
         * Returns the start index of the substring.
         */
        inline int runQuery(int l, int k) noexcept {
            profiler.startNewQuery();
            if constexpr (Debug) std::cout << "Running topk query with l = " << l << " and k = " << k << std::endl;

            profiler.startCollectCandidates();
            std::vector<Candidate> candidates;
            //Avoid reallocation by reserving for as many candidates as are possible.
            candidates.reserve(tree->n);
            //Collect all relevant candidates for the given length.
            collectingBfs(candidates, l);
            //The reserve() call above typically increases the vector size unnecessarily (by a lot).
            //My evaluation shows that this slows down the following stable_sort (below); I assume that that is because more memory is allocated internally (?).
            //Either way, the real size that is needed is now known, so I just let the vector know it does not need to be larger any longer to save time later.
            candidates.shrink_to_fit();
            profiler.endCollectCandidates();

            profiler.startSortCandidates();
            //Candidates now has entries (#occurences, starting position). These were added in suffix tree order and therefore, they are lexicographically sorted.
            //By stable-sorting them by the #occurences (only, i.e. not also by the starting position!) we get them ordered by #occurences.
            //Due to stable-sort and the previous lexicographic order, lexicographically smaller elements are favored in case of equal #occurences.
            //Of course, it is not really necessary to sort the entire vector. It would be theoretically more efficient to use a top-k max heap, but
            //my profiling shows that this step only takes around 5% of the query time, so I chose not to do that.
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

            profiler.startReconstructSolution();//This phase is now pretty useless; there used to be more work to do here...
            //Finally, select the k-th element (0-indexed, obviously) as the final result.
            Candidate& solution = candidates[k - 1];

            if constexpr (Debug) {
                int endIndex = solution.startPosition + l;//inclusively
                std::cout << "Found suffix (" << solution.startPosition << ", " << endIndex << ") with #occ: " << solution.occurences << std::endl;
            }
            profiler.endReconstructSolution();
            profiler.endCurrentQuery();

            //Return the result.
            return solution.startPosition;
        }

        /**
         * Collects all relevant candidates from the suffix tree for the given length.
         *
         * Main idea:
         *  - Traverse the tree with a bfs.
         *  - Find all *highest* nodes that represent string depths >= length.
         *    These are candidates for the solution. Add a candidate with the precomputed number of leaves below them accordingly.
         *  - After a candidate has been added, no node below it needs to be considered, since suffixes there are already represented in other nodes
         *    that are scanned.
         *  - If (and only if, see above) a node is scanned that does not have sufficient string depth yet, all its children need to be explored as well.
         *
         * Actually, the candidates could also be precomputed once for all queries (along with their suffixDepths) and then be filtered
         * for each query. However, that is significantly slower than this approach for small values for l since here, we can often end the search
         * early and do not need to consider as many candidates in the first place.
         */
        inline void collectingBfs(std::vector<Candidate>& candidates, const int length) const noexcept {
            //Use a queue to preserve the suffix ordering from the suffix tree. This is necessary to get lexicographic ordering.
            std::queue<SuffixTree::Node<CharType>*> queue;
            queue.push(&tree->root);
            while (!queue.empty()) {
                const SuffixTree::Node<CharType>* node = queue.front();
                queue.pop();
                if (node->stringDepth >= length && node->representedSuffix + length < tree->n) {
                    //If this node has at least level l and the suffix is valid, add the relevant candidate.
                    //Store #occurences to find the correct entry later on and the representedSuffix to reconstruct the solution.
                    candidates.emplace_back(node->numberOfLeaves, node->representedSuffix);
                } else {
                    //If this path does not have sufficient string depth yet, explore it further. Add all children to the queue.
                    //If we are at a leaf with string depth < length, nothing needs to be done.
                    for (const auto & [key, child] : node->children) {
                        queue.push(child);
                    }
                }
            }
        }

        /**
         * Initial recursion call for additional preprocessing for the suffix tree.
         */
        inline void countNumberOfLeaves() noexcept {
            //Start dfs at root with depth 0.
            countingDfs(&tree->root, 0);
        }

        /**
         * Actual recursive dfs to calculate the following:
         *  - string depth of every node
         *  - representedSuffix of all internal nodes. These are precomputed by the suffix tree computation for all leaves (needed for the repeat query).
         *    Here, I reuse this entry for internal nodes to precompute a possible represented suffix
         *    (up to the given length, all leaves are equal, so it does not matter which one) to save some time in the actual queries.
         *  - numberOfLeaves, the number of leaves in the subtree rooted at every node. This is calculated recursively by propagating the lower nodes' values upwards.
         *
         *  Returns numberOfLeaves.
         */
        inline int countingDfs(SuffixTree::Node<CharType>* node, int depth) noexcept {
            //This node has stringDepth of depth + its own length.
            node->stringDepth = depth + *node->endIndex - node->startIndex;
            //calculate a possible suffix that is represented by this node.
            //endIndex - stringDepth is the start position of one of the suffixes represented by leaves below this node.
            node->representedSuffix = *node->endIndex - node->stringDepth;
            if (node->hasChildren()) {
                //remove all sentinel leaves, but count them as children because they represent suffixes, too.
                node->numberOfLeaves = node->children.erase(Sentinel);
                //Recursive dfs calls for all children. Simultaneously, calculate the number of leaves below this node.
                for (const auto & [key, child] : node->children) {
                    node->numberOfLeaves += countingDfs(child, node->stringDepth);
                }
                return node->numberOfLeaves;
            } else {
                //This is a leaf. representedSuffix is already set by the suffix tree generator.
                node->numberOfLeaves = 1;
                return 1;
            }
        }

    public:
        //The suffix tree.
        SuffixTree::SuffixTree<CharType, Debug>* tree;

        //Used solely for optimization.
        Profiler profiler;
    };
}
