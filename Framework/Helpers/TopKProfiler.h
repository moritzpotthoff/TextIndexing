#include <chrono>

#include "Timer.h"

namespace Query {
    class TopKNoProfiler {
    public:
        inline void startInitialization() const noexcept {}
        inline void endInitialization() const noexcept {}
        inline void startNewQuery() const noexcept {}
        inline void endCurrentQuery() const noexcept {}
        inline void startCollectCandidates() const noexcept {}
        inline void endCollectCandidates() const noexcept {}
        inline void startSortCandidates() const noexcept {}
        inline void endSortCandidates() const noexcept {}
        inline void startReconstructSolution() const noexcept {}
        inline void endReconstructSolution() const noexcept {}
        inline void print() const noexcept {}
    };

    class TopKProfiler {
    public:
        TopKProfiler() :
            initializationTime(0),
            totalQueryTime(0),
            collectCandidatesTime(0),
            sortCandidatesTime(0),
            reconstructSolutionTime(0),
            numberOfQueries(0) {}

        inline void startInitialization() noexcept {
            initializationTimer.restart();
        }

        inline void endInitialization() noexcept {
            initializationTime = initializationTimer.getMilliseconds();
        }

        inline void startNewQuery() noexcept {
            numberOfQueries++;
            totalQueryTimer.restart();
        }

        inline void endCurrentQuery() noexcept {
            totalQueryTime += totalQueryTimer.getMilliseconds();
        }

        inline void startCollectCandidates() noexcept {
            collectCandidatesTimer.restart();
        }

        inline void endCollectCandidates() noexcept {
            collectCandidatesTime += collectCandidatesTimer.getMilliseconds();
        }

        inline void startSortCandidates() noexcept {
            sortCandidatesTimer.restart();
        }

        inline void endSortCandidates() noexcept {
            sortCandidatesTime += sortCandidatesTimer.getMilliseconds();
        }

        inline void startReconstructSolution() noexcept {
            reconstructSolutionTimer.restart();
        }

        inline void endReconstructSolution() noexcept {
            reconstructSolutionTime += reconstructSolutionTimer.getMilliseconds();
        }

        inline void print() const noexcept {
            std::cout << "TopK Query evaluation run for " << numberOfQueries << " queries." << std::endl;
            std::cout << "  Initialization time:    " << initializationTime << "ms" << std::endl;
            std::cout << "  Avg. total query time:  " << totalQueryTime / (double) numberOfQueries << "ms" << std::endl;
            std::cout << "    Collect candidates:   " << collectCandidatesTime / (double) numberOfQueries << "ms" << std::endl;
            std::cout << "    Sort candidates:      " << sortCandidatesTime / (double) numberOfQueries << "ms" << std::endl;
            std::cout << "    Reconstruct solution: " << reconstructSolutionTime / (double) numberOfQueries << "ms" << std::endl;
        }

    private:
        Helpers::Timer initializationTimer;
        Helpers::Timer totalQueryTimer;
        Helpers::Timer collectCandidatesTimer;
        Helpers::Timer sortCandidatesTimer;
        Helpers::Timer reconstructSolutionTimer;

        size_t initializationTime;
        size_t totalQueryTime;
        size_t collectCandidatesTime;
        size_t sortCandidatesTime;
        size_t reconstructSolutionTime;

        size_t numberOfQueries;
    };
}