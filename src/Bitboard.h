#pragma once

#define NOMINMAX
#include <cstdint>
#include <cstring>
#include <intrin.h>


struct alignas(32) Bitboard {
    static constexpr int WIDTH = 25;
    static constexpr int HEIGHT = 25;
    static constexpr int CENTER_X = 12;
    static constexpr int CENTER_Y = 13;
    static constexpr const char* META_NAME = "Bitboard_Meta";

    uint32_t data[34]{};

    void clear() {
        std::memset(data, 0, sizeof(data));
    }

    bool isEmpty() const {
        for (int i = 1; i <= HEIGHT; ++i) {
            if (data[i]) return false;
        }
        return true;
    }

    int popcount() const {
        int count = 0;
        for (int i = 1; i <= HEIGHT; ++i) {
            count += __popcnt(data[i]);
        }
        return count;
    }

    void setCell(int x, int y, bool value) {
        if (y < 1 || y > HEIGHT || x < 0 || x > WIDTH - 1) return;
        if (value) {
            data[y] |= (1U << x);
        } else {
            data[y] &= ~(1U << x);
        }
    }

    bool getCell(int x, int y) const {
        if (y < 1 || y > HEIGHT || x < 0 || x > WIDTH - 1) return false;
        return (data[y] & (1U << x)) != 0;
    }

    void merge(const Bitboard& other) {
        for (int i = 1; i <= HEIGHT; ++i) {
            data[i] |= other.data[i];
        }
    }

    void applyObstacles(const Bitboard& obstacles) {
        for (int i = 1; i <= HEIGHT; ++i) {
            data[i] &= ~obstacles.data[i];
        }
    }
};