#pragma once
#define NOMINMAX
#include <cstdint>
#include <cstring>
#include <intrin.h>

struct alignas(32) Bitboard_25 {
    uint32_t data[34]{};

    void clear() {
        std::memset(data, 0, sizeof(data));
    }

    bool isEmpty() const {
        for (int i = 1; i <= 25; ++i) {
            if (data[i]) return false;
        }
        return true;
    }

    int popcount() const {
        int count = 0;
        for (int i = 1; i <= 25; ++i) {
            count += __popcnt(data[i]);
        }
        return count;
    }

    void merge(const Bitboard_25& other) {
        for (int i = 1; i <= 25; ++i) {
            data[i] |= other.data[i];
        }
    }

    void applyObstacles(const Bitboard_25& obstacles) {
        for (int i = 1; i <= 25; ++i) {
            data[i] &= ~obstacles.data[i];
        }
    }

    void setCell(int x, int y, bool value) {
        if (y < 1 || y > 25 || x < 0 || x > 24) return;
        if (value) {
            data[y] |= (1U << x);
        }
        else {
            data[y] &= ~(1U << x);
        }
    }

    bool getCell(int x, int y) const {
        if (y < 1 || y > 25 || x < 0 || x > 24) return false;
        return (data[y] & (1U << x)) != 0;
    }
};

struct alignas(16) Row_75 {
    uint64_t low;
    uint64_t high;
};

struct alignas(32) Bitboard_75 {
    Row_75 rows[84]{};

    void clear() {
        std::memset(rows, 0, sizeof(rows));
    }

    bool isEmpty() const {
        for (int i = 1; i <= 75; ++i) {
            if (rows[i].low || rows[i].high) return false;
        }
        return true;
    }

    int popcount() const {
        int count = 0;
        for (int i = 1; i <= 75; ++i) {
            count += __popcnt64(rows[i].low);
            count += __popcnt64(rows[i].high);
        }
        return count;
    }

    void merge(const Bitboard_75& other) {
        for (int i = 1; i <= 75; ++i) {
            rows[i].low |= other.rows[i].low;
            rows[i].high |= other.rows[i].high;
        }
    }

    void applyObstacles(const Bitboard_75& obstacles) {
        for (int i = 1; i <= 75; ++i) {
            rows[i].low &= ~obstacles.rows[i].low;
            rows[i].high &= ~obstacles.rows[i].high;
        }
    }

    void setCell(int x, int y, bool value) {
        if (y < 1 || y > 75 || x < 0 || x > 74) return;
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

    bool getCell(int x, int y) const {
        if (y < 1 || y > 75 || x < 0 || x > 74) return false;
        if (x < 64) {
            return (rows[y].low & (1ULL << x)) != 0;
        }
        else {
            return (rows[y].high & (1ULL << (x - 64))) != 0;
        }
    }
};