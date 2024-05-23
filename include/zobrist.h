#pragma once
#include <array>
#include <cstdint>
#include <unordered_map>
#include <random>

#include "board/board.h"

class Board; // fwd declaration

constexpr int kNumSquares = 64;
constexpr int kNumPieces = 12;
constexpr int kNumCastling = 4;

namespace Zobrist {
    extern std::array<std::array<uint64_t, kNumSquares>, kNumPieces> pieceKeys;
    extern uint64_t blackToMove;
    extern std::array<uint64_t, kNumCastling> castlingKeys;
    extern std::array<uint64_t, kNumSquares> enPassantKeys;
    extern std::unordered_map<uint64_t, uint64_t> table;

    void initialize();
    uint64_t computeHash(const Board& board);
    inline uint64_t updateHash(uint64_t hash, const Move& move, const Board& board);
}
