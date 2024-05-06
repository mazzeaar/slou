#include "zobrist.h"

namespace Zobrist {
    std::array<std::array<u64, kNumSquares>, kNumPieces> pieceKeys;

    u64 blackToMove;
    std::array<u64, kNumCastling> castlingKeys;
    std::array<u64, kNumSquares> enPassantKeys;

    std::unordered_map<uint64_t, uint64_t> table;

    void initialize()
    {
        std::mt19937_64 rng(0xdeadbeef); // Fixed seed for reproducibility
        std::uniform_int_distribution<u64> dist(0, std::numeric_limits<u64>::max());

        for ( int piece = 0; piece < kNumPieces; ++piece ) {
            for ( int square = 0; square < kNumSquares; ++square ) {
                pieceKeys[piece][square] = dist(rng);
            }
        }

        blackToMove = dist(rng);

        for ( int i = 0; i < kNumCastling; ++i ) {
            castlingKeys[i] = dist(rng);
        }

        for ( int square = 0; square < kNumSquares; ++square ) {
            enPassantKeys[square] = dist(rng);
        }
    }

    u64 computeHash(const Board& board)
    {
        u64 hash = 0;

        for ( int square = 0; square < kNumSquares; ++square ) {
            int piece_id = board.getIndex(board.getPieceFromSquare(square));
            if ( piece_id != board.getIndex(type::Piece::none) ) {
                hash ^= pieceKeys[piece_id][square];
            }
        }

        if ( !board.whiteTurn() ) {
            hash ^= blackToMove;
        }

        if ( board.canCastleKs(type::Color::white) ) hash ^= castlingKeys[0];
        if ( board.canCastleKs(type::Color::white) ) hash ^= castlingKeys[1];
        if ( board.canCastleQs(type::Color::black) ) hash ^= castlingKeys[2];
        if ( board.canCastleQs(type::Color::black) ) hash ^= castlingKeys[3];

        if ( board.getEpField() != 0ULL ) {
            hash ^= enPassantKeys[get_LSB(board.getEpField())];
        }

        return hash;
    }

}; // namespace zobrist