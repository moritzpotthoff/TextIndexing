#include <chrono>

#include "Timer.h"

namespace Query {
    class RepeatNoProfiler {
    public:
        inline void startStringDepth() const noexcept {}
        inline void endStringDepth() const noexcept {}
        inline void startCollectInnerNodes() const noexcept {}
        inline void endCollectInnerNodes() const noexcept {}
        inline void startActualQuery() const noexcept {}
        inline void endActualQuery() const noexcept {}
        inline void startInnerNodePhase() const noexcept {}
        inline void endInnerNodePhase() const noexcept {}
        inline void startPairPhase() const noexcept {}
        inline void endPairPhase() const noexcept {}
        inline void print() const noexcept {}
        inline void startMergePhase() const noexcept {}
        inline void endMergePhase() const noexcept {}
    };

    class RepeatProfiler {
    public:
        RepeatProfiler() :
                stringDepthTime(0),
                collectInnerNodesTime(0),
                actualQueryTime(0),
                totalInnerNodePhaseTime(0),
                totalPairTime(0),
                numberOfInnerNodePhases(0),
                numberOfPairCalls(0) {}

        inline void startStringDepth() noexcept {
            stringDepthTimer.restart();
        }

        inline void endStringDepth() noexcept {
            stringDepthTime = stringDepthTimer.getMilliseconds();
        }

        inline void startCollectInnerNodes() noexcept {
            collectInnerNodesTimer.restart();
        }

        inline void endCollectInnerNodes() noexcept {
            collectInnerNodesTime = collectInnerNodesTimer.getMilliseconds();
        }

        inline void startActualQuery() noexcept {
            actualQueryTimer.restart();
        }

        inline void endActualQuery() noexcept {
            actualQueryTime = actualQueryTimer.getMilliseconds();
        }

        inline void startInnerNodePhase() noexcept {
            numberOfInnerNodePhases++;
            innerNodePhaseTimer.restart();
        }

        inline void endInnerNodePhase() noexcept {
            totalInnerNodePhaseTime += innerNodePhaseTimer.getMicroseconds();
        }

        inline void startPairPhase() noexcept {
            numberOfPairCalls++;
            pairTimer.restart();
        }

        inline void endPairPhase() noexcept {
            totalPairTime += pairTimer.getMicroseconds();
        }

        inline void startMergePhase() noexcept {
            numberOfMerges++;
            mergeTimer.restart();
        }

        inline void endMergePhase() noexcept {
            totalMergeTime += mergeTimer.getMicroseconds();
        }

        inline void print() const noexcept {
            size_t totalTime = stringDepthTime + collectInnerNodesTime + actualQueryTime;
            std::cout << "Repeat Query evaluation run. Total time: " << totalTime << "ms" << std::endl;
            std::cout << "  Total init. time:         " << (stringDepthTime + collectInnerNodesTime) << "ms" << std::endl;
            std::cout << "    -- string depth:        " << stringDepthTime << "ms" << std::endl;
            std::cout << "    -- collect inner nodes: " << collectInnerNodesTime << "ms" << std::endl;
            std::cout << "  Total actual query time:  " << actualQueryTime << "ms" << std::endl;
            std::cout << "    -- inner node phases:   " << totalInnerNodePhaseTime/1000 << "ms (" << numberOfInnerNodePhases << " * " << (totalInnerNodePhaseTime / (double) numberOfInnerNodePhases) << "microseconds)." << std::endl;
            std::cout << "    -- pair calls:          " << totalPairTime/1000 << "ms (" << numberOfPairCalls << " * " << (totalPairTime / (double) numberOfPairCalls) << "microseconds)." << std::endl;
            std::cout << "    -- merge time:          " << totalMergeTime/1000 << "ms (" << numberOfMerges << " * " << (totalMergeTime / (double) numberOfMerges) << "microseconds)." << std::endl;
        }

    private:
        Helpers::Timer stringDepthTimer;
        Helpers::Timer collectInnerNodesTimer;
        Helpers::Timer actualQueryTimer;
        Helpers::Timer innerNodePhaseTimer;
        Helpers::Timer pairTimer;
        Helpers::Timer mergeTimer;

        size_t stringDepthTime;
        size_t collectInnerNodesTime;
        size_t actualQueryTime;
        size_t totalInnerNodePhaseTime;
        size_t totalPairTime;
        size_t totalMergeTime;

        size_t numberOfInnerNodePhases;
        size_t numberOfPairCalls;
        size_t numberOfMerges;
    };
}