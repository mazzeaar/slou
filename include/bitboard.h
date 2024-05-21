#pragma once

#include <cstdint>
#include <iostream>
#include <sstream>

typedef uint64_t u64;

enum Directions {
    North = 8,
    South = -8,
    East = 1,
    West = -1,
    NorthWest = North + West,
    NorthEast = North + East,
    SouthWest = South + West,
    SouthEast = South + East
};

#define single_bit_u64(i)   ((1ULL << (i)))
#define set_bit(b, i)       ((b) |= single_bit_u64(i))
#define get_bit(b, i)       ((b) & (1ULL << (i)))
#define clear_bit(b, i)     ((b) &= ~(1ULL << (i)))
#define get_LSB(b)          (__builtin_ctzll(b))
#define get_bit_count(b)    (__builtin_popcountll(b))

const u64 RANK_1 = 0x00000000000000FFULL;
const u64 RANK_2 = 0x000000000000FF00ULL;
const u64 RANK_3 = 0x0000000000FF0000ULL;
const u64 RANK_4 = 0x00000000FF000000ULL;
const u64 RANK_5 = 0x000000FF00000000ULL;
const u64 RANK_6 = 0x0000FF0000000000ULL;
const u64 RANK_7 = 0x00FF000000000000ULL;
const u64 RANK_8 = 0xFF00000000000000ULL;
const u64 RANK_12 = RANK_1 | RANK_2;
const u64 RANK_78 = RANK_7 | RANK_8;

const u64 FILE_A = 0x0101010101010101ULL;
const u64 FILE_B = 0x0202020202020202ULL;
const u64 FILE_C = 0x0404040404040404ULL;
const u64 FILE_D = 0x0808080808080808ULL;
const u64 FILE_E = 0x1010101010101010ULL;
const u64 FILE_F = 0x2020202020202020ULL;
const u64 FILE_G = 0x4040404040404040ULL;
const u64 FILE_H = 0x8080808080808080ULL;
const u64 FILE_AB = FILE_A | FILE_B;
const u64 FILE_GH = FILE_G | FILE_H;

const u64 NULL_BB = 0ULL;
const u64 FULL_BB = 0xFFFFFFFFFFFFFFFFULL;

// +8
inline u64 north(u64 b) { return b << 8; }
// -8
inline u64 south(u64 b) { return b >> 8; }
// +1
inline u64 east(u64 b) { return (b & ~FILE_H) << 1; }
// -1
inline u64 west(u64 b) { return (b & ~FILE_A) >> 1; }
// +1
inline u64 unsafe_east(u64 b) { return b << 1; }
// -1
inline u64 unsafe_west(u64 b) { return b >> 1; }

// +7
inline u64 north_west(u64 b) { return west(north(b)); }
// +9
inline u64 north_east(u64 b) { return east(north(b)); }
// -7
inline u64 south_east(u64 b) { return east(south(b)); }
// -9
inline u64 south_west(u64 b) { return west(south(b)); }

// +7
inline u64 unsafe_north_west(u64 b) { return unsafe_west(north(b)); }
// +9
inline u64 unsafe_north_east(u64 b) { return unsafe_east(north(b)); }
// -7
inline u64 unsafe_south_east(u64 b) { return unsafe_east(south(b)); }
// -9
inline u64 unsafe_south_west(u64 b) { return unsafe_west(south(b)); }

inline u64 extract_next_bit(u64& bb)
{
    const u64 lsb = bb & -bb;
    bb &= bb - 1;
    return lsb;
}

inline constexpr int pop_LSB(u64& b)
{
    int i = get_LSB(b);
    b &= b - 1;
    return i;
}

inline u64 pop_lsb_to_u64(u64& b)
{
    u64 ret = b & -b;       // isolate lsb
    b &= b - 1;             // remove lsb
    return ret;
}

inline u64 random_u64()
{
    return ((u64) (random()) & 0xFFFF)
        | (((u64) (random()) & 0xFFFF) << 16)
        | (((u64) (random()) & 0xFFFF) << 32)
        | (((u64) (random()) & 0xFFFF) << 48);
}

inline u64 random_u64_fewbits()
{
    return random_u64() & random_u64() & random_u64();
}

inline void print_bb(u64 b)
{
    std::stringstream ss;

    // some fuckery because of how the board is stored
    for ( int i = 7; i >= 0; --i ) {
        for ( int j = 0; j < 8; ++j ) {
            int idx = (8 * i) + j;
            ss << (bool) ((b >> idx) & 0x1);
        }
        ss << '\n';
    }

    std::cout << ss.str() << std::endl;
}

template <typename T>
void print_bb(T& title, const u64& b)
{
    std::cout << title << ": " << std::endl;
    print_bb(b);
}