#pragma once

#define NOMINMAX
#include <cstdint>
#include <cstring>
#include <intrin.h>

#include "../core/IBitboard.h"


struct alignas(16) Row_75 {
    uint64_t low;
    uint64_t high;
};

struct alignas(32) Bitboard_75 : public IBitboard {
    static constexpr int WIDTH = 75;
    static constexpr int HEIGHT = 75;
    static constexpr int CENTER_X = 37;
    static constexpr int CENTER_Y = 38;

    static constexpr const char* META_NAME = "Bitboard_75_Meta";

    Row_75 rows[84]{};

    void clear() override {
        std::memset(rows, 0, sizeof(rows));
    }

    bool isEmpty() const override {
        for (int i = 1; i <= HEIGHT; ++i) {
            if (rows[i].low || rows[i].high) return false;
        }
        return true;
    }

    int popcount() const override {
        int count = 0;
        for (int i = 1; i <= HEIGHT; ++i) {
            count += __popcnt64(rows[i].low);
            count += __popcnt64(rows[i].high);
        }
        return count;
    }

    void setCell(int x, int y, bool value) override {
        if (y < 1 || y > HEIGHT || x < 0 || x > WIDTH - 1) 
            return;
        if (x < 64) {
            if (value) {
                rows[y].low |= (1ULL << x);
            }
            else {
                rows[y].low &= ~(1ULL << x);
            }
        }
        else {
            int shift = x - 64;
            if (value) {
                rows[y].high |= (1ULL << shift);
            }
            else {
                rows[y].high &= ~(1ULL << shift);
            }
        }
    }

    bool getCell(int x, int y) const override {
        if (y < 1 || y > HEIGHT || x < 0 || x > WIDTH - 1) 
            return false;
        if (x < 64) {
            return (rows[y].low & (1ULL << x)) != 0;
        }
        else {
            return (rows[y].high & (1ULL << (x - 64))) != 0;
        }
    }

    void merge(const IBitboard& other) override {
        const Bitboard_75& src = static_cast<const Bitboard_75&>(other);
        for (int i = 1; i <= HEIGHT; ++i) {
            rows[i].low |= src.rows[i].low;
            rows[i].high |= src.rows[i].high;
        }
    }

    void applyObstacles(const IBitboard& obstacles) override {
        const Bitboard_75& obs = static_cast<const Bitboard_75&>(obstacles);
        for (int i = 1; i <= HEIGHT; ++i) {
            rows[i].low &= ~obs.rows[i].low;
            rows[i].high &= ~obs.rows[i].high;
        }
    }
};