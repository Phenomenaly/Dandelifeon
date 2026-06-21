#pragma once

#define NOMINMAX
#include <immintrin.h>

// #include "../core/IBitboard.h"
#include "Bitboard_25.h"


class DandelifeonEngine_25 {
private:
    __m256i row_mask;

public:
    DandelifeonEngine_25() {
        row_mask = _mm256_set1_epi32(0x1FFFFFF);
    }

    void step(const Bitboard_25& current, Bitboard_25& next, const Bitboard_25& obstacles) const {
        for (int i = 1; i <= 25; i += 8) {
            __m256i mid = _mm256_loadu_si256((const __m256i*) & current.data[i]);
            __m256i top = _mm256_loadu_si256((const __m256i*) & current.data[i - 1]);
            __m256i bot = _mm256_loadu_si256((const __m256i*) & current.data[i + 1]);

            __m256i n1 = _mm256_slli_epi32(top, 1); __m256i n2 = top;   __m256i n3 = _mm256_srli_epi32(top, 1);
            __m256i n4 = _mm256_slli_epi32(mid, 1);                     __m256i n5 = _mm256_srli_epi32(mid, 1);
            __m256i n6 = _mm256_slli_epi32(bot, 1); __m256i n7 = bot;   __m256i n8 = _mm256_srli_epi32(bot, 1);
            

            __m256i s0 = _mm256_setzero_si256();
            __m256i s1 = _mm256_setzero_si256();
            __m256i s2 = _mm256_setzero_si256();

            auto add = [&](__m256i x) {
                __m256i c0 = _mm256_and_si256(s0, x);
                s0 = _mm256_xor_si256(s0, x);
                __m256i c1 = _mm256_and_si256(s1, c0);
                s1 = _mm256_xor_si256(s1, c0);
                s2 = _mm256_or_si256(s2, c1);
                };

            add(n1);    add(n2);    add(n3); 
            add(n4);                add(n5); 
            add(n6);    add(n7);    add(n8);

            __m256i res = _mm256_and_si256(_mm256_andnot_si256(s2, s1), _mm256_or_si256(s0, mid));

            __m256i obs_mask = _mm256_loadu_si256((const __m256i*) & obstacles.data[i]);
            res = _mm256_andnot_si256(obs_mask, res);

            _mm256_storeu_si256((__m256i*) & next.data[i], _mm256_and_si256(res, row_mask));
        }
    }
};