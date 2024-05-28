#pragma once

#include <array>
#include <cstdint>
#include <unordered_map>
#include <random>

#include "definitions.h"

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

    inline char last_castling_rights;

    void initialize();
    uint64_t computeHash(const Board& board);

    inline void togglePiece(uint64_t& hash, int piece_id, int square) { hash ^= pieceKeys[piece_id][square]; }

    template <Color color>
    constexpr void toggleCastling(uint64_t& hash)
    {
        if constexpr ( utils::isWhite(color) ) {
            hash ^= castlingKeys[0];
            hash ^= castlingKeys[1];
        }
        else {
            hash ^= castlingKeys[2];
            hash ^= castlingKeys[3];
        }
    }

    template <Color color>
    constexpr void toggleCastlingQs(uint64_t& hash)
    {
        if constexpr ( utils::isWhite(color) ) {
            hash ^= castlingKeys[1];
        }
        else {
            hash ^= castlingKeys[3];
        }
    }

    template <Color color>
    constexpr void toggleCastlingKs(uint64_t& hash)
    {
        if constexpr ( utils::isWhite(color) ) {
            hash ^= castlingKeys[0];
        }
        else {
            hash ^= castlingKeys[2];
        }
    }

    inline void toggleEnPassant(uint64_t& hash, uint64_t ep_field) { hash ^= enPassantKeys[get_LSB(ep_field)]; }
    inline void toggleBlackToMove(uint64_t& hash) { hash ^= blackToMove; }
};
