#pragma once
#include <chrono>

struct Timer {
    void update() {
        // set timestamp and calculate duration
        clock::time_point current = clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(current - previous).count();
        previous = current;

        // convert to useful metric
        delta = (float)microseconds;
        delta *= 0.000001f; // microseconds to seconds
    }

    float get_fps() {
        return 1.0f / delta;
    }
    float get_delta() {
        return delta;
    }

private:
    typedef std::chrono::high_resolution_clock clock;
    clock::time_point previous = clock::now();
    float delta = 0.0f;
};