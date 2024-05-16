#pragma once

#include "bitboard.h"
#include "definitions.h"
#include "log.h"

#include <iostream>
#include <fstream>
#include <array>

namespace magic {
    /**
     * @brief Holds the neccessary data for the magic numbers to work.
     * Using this we can easily look up the possible moves for a given position and occupancy.
     *
     */
    struct Magic {
        std::array<u64, 4096> attack_table;         // pointer to attack_table for each particular square
        u64 mask;                                   // to mask relevant squares of both lines (no outer squares)
        u64 magic;                                  // magic 64-bit factor
        int shift;                                  // shift right
    };

    constexpr int numSquares = 64;                  // numer of square in chess lol
    extern const bool initialized_magics;           // are magics already initialized or not?

    extern std::array<Magic, 64> bishop_magics;     // bishop magics for each square
    extern std::array<Magic, 64> rook_magics;       // rook magics for each square

    /**
     * @brief gets called once, when compiling for the first time.
     * Calculates all magic numbers based on:
     *
     * https://www.chessprogramming.org/Magic_Bitboards
     */
    void initMagics();

    /**
     * @brief Can be used to retrieve the correct Magic object for the piece and square
     *
     * @tparam type
     * @param square
     * @return Magic&
     */
    template <PieceType type>
    inline const Magic& getMagics(int square)
    {
        static_assert((type == PieceType::bishop || type == PieceType::rook) && "PieceType is not allowed here!\n");
        if constexpr ( utils::isBishop(type) ) { return bishop_magics[square]; }
        else { return rook_magics[square]; }
    }

}; // namespace magic
