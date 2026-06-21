#pragma once

#define NOMINMAX
#include <immintrin.h>

//#include "../core/IBitboard.h"
#include "Bitboard_75.h"


class DandelifeonEngine_75 {
private:
    __m128i row_mask;

    inline __m128i shiftLeft1(__m128i val) const {
        __m128i carry = _mm_srli_epi64(val, 63);
        carry = _mm_slli_si128(carry, 8);
        __m128i shifted = _mm_slli_epi64(val, 1);
        return _mm_or_si128(shifted, carry);
    }

    inline __m128i shiftRight1(__m128i val) const {
        __m128i carry = _mm_slli_epi64(val, 63);
        carry = _mm_srli_si128(carry, 8);
        __m128i shifted = _mm_srli_epi64(val, 1);
        return _mm_or_si128(shifted, carry);
    }

public:
    DandelifeonEngine_75() {
        row_mask = _mm_set_epi64x(0x00000000000007FFULL, 0xFFFFFFFFFFFFFFFFULL);
    }

    void step(const Bitboard_75& current, Bitboard_75& next, const Bitboard_75& obstacles) const {
        for (int i = 1; i <= 75; ++i) {
            __m128i mid = _mm_load_si128((const __m128i*) & current.rows[i]);
            __m128i top = _mm_load_si128((const __m128i*) & current.rows[i - 1]);
            __m128i bot = _mm_load_si128((const __m128i*) & current.rows[i + 1]);

            __m128i n1 = shiftLeft1(top);   __m128i n2 = top;   __m128i n3 = shiftRight1(top);
            __m128i n4 = shiftLeft1(mid);                       __m128i n5 = shiftRight1(mid);
            __m128i n6 = shiftLeft1(bot);   __m128i n7 = bot;   __m128i n8 = shiftRight1(bot);
            

            __m128i s0 = _mm_setzero_si128();
            __m128i s1 = _mm_setzero_si128();
            __m128i s2 = _mm_setzero_si128();

            auto add = [&](__m128i x) {
                __m128i c0 = _mm_and_si128(s0, x);
                s0 = _mm_xor_si128(s0, x);
                __m128i c1 = _mm_and_si128(s1, c0);
                s1 = _mm_xor_si128(s1, c0);
                s2 = _mm_or_si128(s2, c1);
                };

            add(n1);    add(n2);    add(n3); 
            add(n4);                add(n5); 
            add(n6);    add(n7);    add(n8);

            __m128i res = _mm_and_si128(_mm_andnot_si128(s2, s1), _mm_or_si128(s0, mid));

            __m128i obs_mask = _mm_load_si128((const __m128i*) & obstacles.rows[i]);
            res = _mm_andnot_si128(obs_mask, res);

            _mm_store_si128((__m128i*) & next.rows[i], _mm_and_si128(res, row_mask));
        }
    }
};