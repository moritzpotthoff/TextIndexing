#include <chrono>

#include "Timer.h"

namespace Query {
    class RepeatNoProfiler {
    public:
        inline void startStringDepth() const noexcept {}
        inline void endStringDepth() const noexcept {}
        inline void startCollectSuffixes() const noexcept {}
        inline void endCollectSuffixes() const noexcept {}
        inline void startActualQuery() const noexcept {}
        inline void endActualQuery() const noexcept {}
        inline void startLengthPhase() const noexcept {}
        inline void endLengthPhase() const noexcept {}
        inline void startLcaPhase() const noexcept {}
        inline void endLcaPhase() const noexcept {}
        inline void print() const noexcept {}
    };

    class RepeatProfiler {
    public:
        RepeatProfiler() : 
            stringDepthTime(0),
            collectSuffixTime(0),
            actualQueryTime(0),
            totalLengthPhaseTime(0),
            lowestCommonAncestorTime(0),
            numberOfLengthPhases(0),
            numberOfLcaCalls(0) {}

        inline void startStringDepth() noexcept {
            stringDepthTimer.restart();
        }

        inline void endStringDepth() noexcept {
            stringDepthTime = stringDepthTimer.getMilliseconds();
        }

        inline void startCollectSuffixes() noexcept {
            collectSuffixTimer.restart();
        }

        inline void endCollectSuffixes() noexcept {
            collectSuffixTime = collectSuffixTimer.getMilliseconds();
        }

        inline void startActualQuery() noexcept {
            actualQueryTimer.restart();
        }

        inline void endActualQuery() noexcept {
            actualQueryTime = actualQueryTimer.getMilliseconds();
        }

        inline void startLengthPhase() noexcept {
            numberOfLengthPhases++;
            lengthPhaseTimer.restart();
        }

        inline void endLengthPhase() noexcept {
            totalLengthPhaseTime += lengthPhaseTimer.getMilliseconds();
        }

        inline void startLcaPhase() noexcept {
            numberOfLcaCalls++;
            lowestCommonAncestorTimer.restart();
        }

        inline void endLcaPhase() noexcept {
            lowestCommonAncestorTime += lowestCommonAncestorTimer.getMilliseconds();
        }

        inline void print() const noexcept {
            size_t totalTime = stringDepthTime + collectSuffixTime + actualQueryTime;
            std::cout << "Repeat Query evaluation run. Total time: " << totalTime << "ms" << std::endl;
            std::cout << "  Total init. time:        " << (stringDepthTime + collectSuffixTime) << "ms" << std::endl;
            std::cout << "    -- string depth:       " << stringDepthTime << "ms" << std::endl;
            std::cout << "    -- collect suffixes:   " << collectSuffixTime << "ms" << std::endl;
            std::cout << "  Total actual query time: " << actualQueryTime << "ms" << std::endl;
            std::cout << "    -- length phases:      " << totalLengthPhaseTime << "ms (" << numberOfLengthPhases << " * " << (totalLengthPhaseTime / (double) numberOfLengthPhases) << ")." << std::endl;
            std::cout << "    -- lca phases:         " << lowestCommonAncestorTime << "ms (" << numberOfLcaCalls << " * " << (lowestCommonAncestorTime / (double) numberOfLcaCalls) << ")." << std::endl;
        }

    private:
        Helpers::Timer stringDepthTimer;
        Helpers::Timer collectSuffixTimer;
        Helpers::Timer actualQueryTimer;
        Helpers::Timer lengthPhaseTimer;
        Helpers::Timer lowestCommonAncestorTimer;

        size_t stringDepthTime;
        size_t collectSuffixTime;
        size_t actualQueryTime;
        size_t totalLengthPhaseTime;
        size_t lowestCommonAncestorTime;

        size_t numberOfLengthPhases;
        size_t numberOfLcaCalls;
    };
}