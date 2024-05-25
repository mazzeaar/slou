#pragma once
#include <array>
#include <cstdint>
#include <unordered_map>
#include <random>

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
    inline void toggleCastlingW(uint64_t& hash) { hash ^= castlingKeys[0]; hash ^= castlingKeys[1]; }
    inline void toggleCastlingB(uint64_t& hash) { hash ^= castlingKeys[2]; hash ^= castlingKeys[3]; }
    inline void toggleCastlingKW(uint64_t& hash) { hash ^= castlingKeys[0]; }
    inline void toggleCastlingQW(uint64_t& hash) { hash ^= castlingKeys[1]; }
    inline void toggleCastlingKB(uint64_t& hash) { hash ^= castlingKeys[2]; }
    inline void toggleCastlingQB(uint64_t& hash) { hash ^= castlingKeys[3]; }
    inline void toggleEnPassant(uint64_t& hash, uint64_t ep_field) { hash ^= enPassantKeys[get_LSB(ep_field)]; }
    inline void toggleBlackToMove(uint64_t& hash) { hash ^= blackToMove; }
};
