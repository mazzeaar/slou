#include "board/board.h"
#include "zobrist.h"
#include <limits>

namespace Zobrist {
    std::array<std::array<uint64_t, kNumSquares>, kNumPieces> pieceKeys;
    uint64_t blackToMove;
    std::array<uint64_t, kNumCastling> castlingKeys;
    std::array<uint64_t, kNumSquares> enPassantKeys;
    std::unordered_map<uint64_t, uint64_t> table;

    void initialize()
    {
        //std::mt19937_64 rng(0xdeadbeef); // Fixed seed for reproducibility
        //std::uniform_int_distribution<uint64_t> dist(0, std::numeric_limits<uint64_t>::max());

        for ( auto& pieceArray : pieceKeys ) {
            for ( auto& key : pieceArray ) {
                //key = dist(rng);
                key = random_u64();
            }
        }

        // blackToMove = dist(rng);
        blackToMove = random_u64();

        for ( auto& key : castlingKeys ) {
            // key = dist(rng);
            key = random_u64();

        }

        for ( auto& key : enPassantKeys ) {
            // key = dist(rng);
            key = random_u64();

        }
    }

    uint64_t computeHash(const Board& board)
    {
        uint64_t hash = 0;
        last_castling_rights = board.getRawCastlingRights();

        for ( int square = 0; square < kNumSquares; ++square ) {
            int piece_id = board.getIndex(board.getPiece(square));
            if ( piece_id != board.getIndex(Piece::none) ) {
                hash ^= pieceKeys[piece_id][square];
            }
        }

        if ( !board.whiteTurn() ) {
            hash ^= blackToMove;
        }

        if ( board.canCastleKs<Color::white>() ) hash ^= castlingKeys[0];
        if ( board.canCastleQs<Color::white>() ) hash ^= castlingKeys[1];
        if ( board.canCastleKs<Color::black>() ) hash ^= castlingKeys[2];
        if ( board.canCastleQs<Color::black>() ) hash ^= castlingKeys[3];

        if ( board.getEpField() != 0ULL ) {
            hash ^= enPassantKeys[get_LSB(board.getEpField())];
        }

        return hash;
    }
}; // namespace Zobrist