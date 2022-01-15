#include <chrono>

#include "Timer.h"

namespace Query {
    class NoProfiler {
    public:
        void startInitialization() const noexcept {}
        void endInitialization() const noexcept {}
        void startNewQuery() const noexcept {}
        void endCurrentQuery() const noexcept {}
        void startCollectCandidates() const noexcept {}
        void endCollectCandidates() const noexcept {}
        void startSortCandidates() const noexcept {}
        void endSortCandidates() const noexcept {}
        void startReconstructSolution() const noexcept {}
        void endReconstructSolution() const noexcept {}
        void print() const noexcept {}
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

        void startInitialization() noexcept {
            initializationTimer.restart();
        }

        void endInitialization() noexcept {
            initializationTime = initializationTimer.getMilliseconds();
        }

        void startNewQuery() noexcept {
            numberOfQueries++;
            totalQueryTimer.restart();
        }

        void endCurrentQuery() noexcept {
            totalQueryTime += totalQueryTimer.getMilliseconds();
        }

        void startCollectCandidates() noexcept {
            collectCandidatesTimer.restart();
        }

        void endCollectCandidates() noexcept {
            collectCandidatesTime += collectCandidatesTimer.getMilliseconds();
        }

        void startSortCandidates() noexcept {
            sortCandidatesTimer.restart();
        }

        void endSortCandidates() noexcept {
            sortCandidatesTime += sortCandidatesTimer.getMilliseconds();
        }

        void startReconstructSolution() noexcept {
            reconstructSolutionTimer.restart();
        }

        void endReconstructSolution() noexcept {
            reconstructSolutionTime += reconstructSolutionTimer.getMilliseconds();
        }

        void print() const noexcept {
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