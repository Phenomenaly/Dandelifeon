#pragma once

#define NOMINMAX
#include <cstdint>
#include <cstring>
#include <intrin.h>

#include "../core/IBitboard.h"


struct alignas(32) Bitboard_25 : public IBitboard {
    static constexpr int WIDTH = 25;
    static constexpr int HEIGHT = 25;
    static constexpr int CENTER_X = 12;
    static constexpr int CENTER_Y = 13;

    static constexpr const char* META_NAME = "Bitboard_25_Meta";

    uint32_t data[34]{};

    void clear() override {
        std::memset(data, 0, sizeof(data));
    }

    bool isEmpty() const override {
        for (int i = 1; i <= HEIGHT; ++i) {
            if (data[i]) return false;
        }
        return true;
    }

    int popcount() const override {
        int count = 0;
        for (int i = 1; i <= HEIGHT; ++i) {
            count += __popcnt(data[i]);
        }
        return count;
    }

    void setCell(int x, int y, bool value) override {
        if (y < 1 || y > HEIGHT || x < 0 || x > WIDTH - 1) return;
        if (value) {
            data[y] |= (1U << x);
        }
        else {
            data[y] &= ~(1U << x);
        }
    }

    bool getCell(int x, int y) const override {
        if (y < 1 || y > HEIGHT || x < 0 || x > WIDTH - 1) return false;
        return (data[y] & (1U << x)) != 0;
    }

    void merge(const IBitboard& other) override {
        const Bitboard_25& src = static_cast<const Bitboard_25&>(other);
        for (int i = 1; i <= HEIGHT; ++i) {
            data[i] |= src.data[i];
        }
    }

    void applyObstacles(const IBitboard& obstacles) override {
        const Bitboard_25& obs = static_cast<const Bitboard_25&>(obstacles);
        for (int i = 1; i <= HEIGHT; ++i) {
            data[i] &= ~obs.data[i];
        }
    }
};