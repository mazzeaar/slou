#pragma once

#include <array>
#include <random>
#include "board.h"
#include "definitions.h"
#include <unordered_map>
#include <map>

typedef uint64_t u64;
#define table_size  4096 * 16

namespace Zobrist {
    inline const int kNumSquares = 64;
    inline const int kNumPieces = 12;
    inline const int kNumPlayers = 2;
    inline const int kNumCastling = 4;

    // static std::array<uint64_t, table_size> table = { 0 };
    extern std::unordered_map<uint64_t, uint64_t> table;
    // Zobrist hash tables
    extern std::array<std::array<u64, kNumSquares>, kNumPieces> pieceKeys;
    extern u64 blackToMove;
    extern std::array<u64, kNumCastling> castlingKeys;
    extern std::array<u64, kNumSquares> enPassantKeys;

    // Functions
    void initialize();
    u64 computeHash(const Board& board);

    inline void insert(uint64_t hash, uint64_t count)
    {
        // table[hash] = count;
        auto [it, inserted] = table.try_emplace(hash, count);
        if ( !inserted ) {
            // std::cerr << "Hash collision or duplicate insertion detected at hash: " << hash << std::endl;
        }
    }

    inline bool has(uint64_t hash, uint64_t& count)
    {
        auto it = table.find(hash);
        if ( it != table.end() ) {
            count = it->second;
            return true;
        }

        return false;
    }

    inline void clear() { table.clear(); }
}
