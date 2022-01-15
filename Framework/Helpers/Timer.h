#include <chrono>

namespace Helpers {

    class Timer {
    public:
        Timer() : start(std::chrono::high_resolution_clock::now()) {}

        inline void restart() noexcept {
            start = std::chrono::high_resolution_clock::now();
        }

        inline size_t getMilliseconds() {
            std::chrono::time_point<std::chrono::high_resolution_clock> end = std::chrono::high_resolution_clock::now();
            return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> start;
    };
}