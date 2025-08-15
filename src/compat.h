#pragma once
#include <cstdint>
#undef NDEBUG
#include <cassert>
#if defined(_M_X64) || defined(__x86_64__)
#include <immintrin.h>
#endif

#ifdef _MSC_VER
#include <cstdlib>
#define FORCEINLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define FORCEINLINE [[gnu::always_inline]] inline
#else
#define FORCEINLINE inline
#endif


namespace intrin {
// count 1 bits
FORCEINLINE uint64_t popcnt(uint64_t x) {
#if defined(_M_X64) || defined(__x86_64__)
    return _mm_popcnt_u64(x);
#elif defined(__GNUC__) || defined(__clang__)
    return __builtin_popcountll(x);
#else
    // adapted from Hacker's Delight 5-1
    x = x - ((x >> 1) & 0x5555555555555555ull);
    x = (x & 0x3333333333333333ull) + ((x >> 2)  & 0x3333333333333333ull);
    x = (x + (x >> 4)) & 0xf0f0f0f0f0f0f0full;
    x = x + (x >> 8);
    x = x + (x >> 16);
    x = x + (x >> 32);
    return x & 0x3full;
#endif
}

// count trailing zeros
FORCEINLINE uint64_t ctz(uint64_t x) {
#if defined(_M_X64) || defined(__x86_64__)
    return _tzcnt_u64(x);
#elif defined(__GNUC__) || defined(__clang__)
    return __builtin_ctzll(x);
#else
    // adapted from Hacker's Delight 5-4
    if (x == 0) {
        return 64;
    }
    uint64_t n = 1;
    if ((x & 0xffffffffull) == 0) {
        n += 32;
        x >>= 32;
    }
    if ((x & 0xffffull) == 0) {
        n += 16;
        x >>= 16;
    }
    if ((x & 0xffull) == 0) {
        n += 8;
        x >>= 8;
    }
    if ((x & 0xfull) == 0) {
        n += 4;
        x >>= 4;
    }
    if ((x & 0x3ull) == 0) {
        n += 2;
        x >>= 2;
    }
    return n - (x & 0x1ull);
#endif
}

// reset/clear lowest bit
FORCEINLINE uint64_t blsr(uint64_t x) {
#if defined(_M_X64) || defined(__x86_64__)
    return _blsr_u64(x);
#else
    return x & (x - 1);
#endif
}

// isolate lowest bit
FORCEINLINE uint64_t blsi(uint64_t x) {
#if defined(_M_X64) || defined(__x86_64__)
    return _blsi_u64(x);
#else
    return x & -x;
#endif
}

/*
Parallel bit extract.
Glamdring should use fancy magic bitboards on ARM instead of PEXT,
but it is intended to run fastest on x64 only and so just emulates PEXT.
*/
FORCEINLINE uint64_t pext(uint64_t x, uint64_t m) {
#if defined(_M_X64) || defined(__x86_64__)
    return _pext_u64(x, m);
#else
    // adapted from Hacker's Delight 7-4
    x &= m; // clear irrelevant bits
    uint64_t mk = ~m << 1; // count 0's to right

    for (uint64_t i = 0; i < 6; i++) {
        uint64_t mp = mk ^ (mk << 1); // parallel prefix
        mp ^= mp << 2;
        mp ^= mp << 4;
        mp ^= mp << 8;
        mp ^= mp << 16;
        mp ^= mp << 32;
        uint64_t mv = mp & m; // bits to move
        m = (m ^ mv) | (mv >> (1ull << i)); // compress m
        uint64_t t = x & mv;
        x = (x ^ t) | (t >> (1ull << i)); // compress x
        mk &= ~mp;
    }
    return x;
#endif
}

// parallel bit deposit
FORCEINLINE uint64_t pdep(uint64_t x, uint64_t m) {
#if defined(_M_X64) || defined(__x86_64__)
    return _pdep_u64(x, m);
#else
    // adapted from Hacker's Delight 7-5
    uint64_t array[6];
    uint64_t m0 = m; // save original mask
    uint64_t mk = ~m << 1; // we will count 0's to right
    for (uint64_t i = 0; i < 6; i++) {
        uint64_t mp = mk ^ (mk << 1); // parallel prefix
        mp ^= mp << 2;
        mp ^= mp << 4;
        mp ^= mp << 8;
        mp ^= mp << 16;
        mp ^= mp << 32;
        uint64_t mv = mp & m; // bits to move
        array[i] = mv;
        m = (m ^ mv) | (mv >> (1ull << i)); // compress m
        mk &= ~mp;
    }
    for (int64_t i = 5; i >= 0; i--) {
        uint64_t mv = array[i];
        uint64_t t = x << (1 << i);
        x = (x & ~mv) | (t & mv);
    }
    return x & m0; // clear irrelevant bits
#endif
}

FORCEINLINE uint16_t byteswap(uint16_t x) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_bswap16(x);
#elif defined(_MSC_VER)
    return _byteswap_ushort(x);
#else
    return (x & 0xff) << 8 | x >> 8;
#endif
}

FORCEINLINE uint32_t byteswap(uint32_t x) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_bswap32(x);
#elif defined(_MSC_VER)
    return _byteswap_ulong(x);
#else
    return (x & 0xff) << 24 | (x & 0xff00) << 8 | (x & 0xff0000) >> 8 | (x & 0xff000000 >> 24);
#endif
}

FORCEINLINE uint64_t byteswap(uint64_t x) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_bswap64(x);
#elif defined(_MSC_VER)
    return _byteswap_uint64(x);
#else
    return (x & 0xff) << 56 | (x & 0xff00) << 40 | (x & 0xff0000) << 24 | (x & 0xff000000) << 8 |
           (x & 0xff00000000) >> 8 | (x & 0xff00000000) >> 24 | (x & 0xff0000000000) >> 40 | (x & 0xff000000000000) >> 56;
#endif
}
}