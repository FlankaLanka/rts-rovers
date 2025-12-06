#pragma once

#include <chrono>

namespace terrafirma {

// Singleton time utility to ensure consistent timestamps across the application
class TimeUtil {
public:
    static double getTime() {
        static auto startTime = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration<double>(now - startTime).count();
    }
};

} // namespace terrafirma

