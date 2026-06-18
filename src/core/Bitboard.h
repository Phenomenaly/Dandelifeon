#pragma once
#define NOMINMAX
#include <cstdint>
#include <cstring>
#include <immintrin.h>
#include <intrin.h>


struct alignas(32) Bitboard_25 {
    uint32_t data[34]{};

    void clear() {
        __m256i* dest = (__m256i*)data;
        __m256i zero = _mm256_setzero_si256();
        dest[0] = zero;
        dest[1] = zero;
        dest[2] = zero;
        dest[3] = zero;
        *(uint64_t*)&data[32] = 0;
    }

    bool isEmpty() const {
        const __m256i* v = (const __m256i*)data;
        __m256i or_all = _mm256_or_si256(
            _mm256_or_si256(v[0], v[1]),
            _mm256_or_si256(v[2], v[3])
        );
        if (!_mm256_testz_si256(or_all, or_all)) {
            return false;
        }

        return *(const uint64_t*)&data[32] == 0;
    }

    int popcount() const {
        const uint64_t* p = (const uint64_t*)data;
        int count = 0;

        for (int i = 0; i < 17; ++i) {
            count += __popcnt64(p[i]);
        }

        return count;
    }

    void merge(const Bitboard_25& other) {
        __m256i* dest = (__m256i*)data;
        const __m256i* src = (const __m256i*)other.data;
        dest[0] = _mm256_or_si256(dest[0], src[0]);
        dest[1] = _mm256_or_si256(dest[1], src[1]);
        dest[2] = _mm256_or_si256(dest[2], src[2]);
        dest[3] = _mm256_or_si256(dest[3], src[3]);
        *(uint64_t*)&data[32] |= *(const uint64_t*)&other.data[32];
    }

    void applyObstacles(const Bitboard_25& obstacles) {
        __m256i* dest = (__m256i*)data;
        const __m256i* obs = (const __m256i*)obstacles.data;
        dest[0] = _mm256_andnot_si256(obs[0], dest[0]);
        dest[1] = _mm256_andnot_si256(obs[1], dest[1]);
        dest[2] = _mm256_andnot_si256(obs[2], dest[2]);
        dest[3] = _mm256_andnot_si256(obs[3], dest[3]);
        *(uint64_t*)&data[32] &= ~*(const uint64_t*)&obstacles.data[32];
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
    Row_75 rows[84];

    void clear() {
        __m256i* dest = (__m256i*)rows;
        __m256i zero = _mm256_setzero_si256();
        for (int i = 0; i < 42; ++i) {
            dest[i] = zero;
        }
    }

    bool isEmpty() const {
        const __m256i* v = (const __m256i*)rows;
        __m256i or_all = _mm256_setzero_si256();
        for (int i = 0; i < 42; ++i) {
            or_all = _mm256_or_si256(or_all, v[i]);
        }
        return _mm256_testz_si256(or_all, or_all);
    }

    int popcount() const {
        const uint64_t* p = (const uint64_t*)rows;
        int count = 0;
        for (int i = 0; i < 168; ++i) {
            count += __popcnt64(p[i]);
        }
        return count;
    }

    void merge(const Bitboard_75& other) {
        __m256i* dest = (__m256i*)rows;
        const __m256i* src = (const __m256i*)other.rows;
        for (int i = 0; i < 42; ++i) {
            dest[i] = _mm256_or_si256(dest[i], src[i]);
        }
    }

    void applyObstacles(const Bitboard_75& obstacles) {
        __m256i* dest = (__m256i*)rows;
        const __m256i* obs = (const __m256i*)obstacles.rows;
        for (int i = 0; i < 42; ++i) {
            dest[i] = _mm256_andnot_si256(obs[i], dest[i]);
        }
    }

    void setCell(int x, int y, bool value) {
        if (y < 1 || y > 75 || x < 0 || x > 74) return;
        if (x < 64) {
            if (value) { rows[y].low |= (1ULL << x); }
            else { rows[y].low &= ~(1ULL << x); }
        }
        else {
            int shift = x - 64;
            if (value) { rows[y].high |= (1ULL << shift); }
            else { rows[y].high &= ~(1ULL << shift); }
        }
    }

    bool getCell(int x, int y) const {
        if (y < 1 || y > 75 || x < 0 || x > 74) return false;
        
        if (x < 64) { return (rows[y].low & (1ULL << x)) != 0; }
        else { return (rows[y].high & (1ULL << (x - 64))) != 0; }
    }

};