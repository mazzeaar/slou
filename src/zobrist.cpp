#include "board/board.h"
#include "zobrist.h"
#include <limits>

namespace Zobrist {
    std::array<std::array<uint64_t, kNumSquares>, kNumPieces> pieceKeys;
    uint64_t blackToMove;
    std::array<uint64_t, kNumCastling> castlingKeys;
    std::array<uint64_t, kNumSquares> enPassantKeys;
    std::unordered_map<uint64_t, uint64_t> table;
}; // namespace Zobrist

void Zobrist::initialize()
    {
        std::mt19937_64 rng(0xdeadbeef); // Fixed seed for reproducibility
        std::uniform_int_distribution<uint64_t> dist(0, std::numeric_limits<uint64_t>::max());

        for ( auto& pieceArray : pieceKeys ) {
            for ( auto& key : pieceArray ) {
                key = dist(rng);
            }
        }

        blackToMove = dist(rng);

        for ( auto& key : castlingKeys ) {
            key = dist(rng);
        }

        for ( auto& key : enPassantKeys ) {
            key = dist(rng);
        }
    }

uint64_t Zobrist::computeHash(const Board& board)
    {
        uint64_t hash = 0;

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

uint64_t Zobrist::updateHash(uint64_t hash, const Move& move, const Board& board)
    {
        int piece_id = board.getIndex(board.getPiece(move.getFrom()));
        hash ^= pieceKeys[piece_id][move.getFrom()];
        hash ^= pieceKeys[piece_id][move.getTo()];


        if ( move.isCapture() ) {
            int captured_piece_id = board.getIndex(board.getPiece(move.getTo()));
            hash ^= pieceKeys[captured_piece_id][move.getTo()];
        }

        // special cases
        if ( move.getFlag() == Move::Flag::castle_k || move.getFlag() == Move::Flag::castle_q ) {
            int rook_from, rook_to;
            if ( move.getFlag() == Move::Flag::castle_k ) {
                rook_from = (board.whiteTurn()) ? 7 : 63;
                rook_to = (board.whiteTurn()) ? 5 : 61;
            }
            else {
                rook_from = (board.whiteTurn()) ? 0 : 56;
                rook_to = (board.whiteTurn()) ? 3 : 59;
            }

            int rook_id = board.getIndex(board.getPiece(rook_from));
            hash ^= pieceKeys[rook_id][rook_from];
            hash ^= pieceKeys[rook_id][rook_to];
        }
        else if ( move.getFlag() == Move::Flag::ep ) {
            int ep_square = (board.whiteTurn()) ? move.getTo() - 8 : move.getTo() + 8;
            int captured_piece_id = board.getIndex(board.getPiece(ep_square));
            hash ^= pieceKeys[captured_piece_id][ep_square];
        }
        else if ( move.isPromotion() ) {
            int promo_id = board.getIndex(utils::getPiece(move.getPromotionPieceType(), board.whiteTurn() ? Color::white : Color::black));
            hash ^= pieceKeys[promo_id][move.getTo()];
        }

        // Toggle the side to move
        hash ^= blackToMove;

        // Update castling rights
        if ( move.isCastle() ) {
            if ( board.canCastleKs<Color::white>() ) hash ^= castlingKeys[0];
            if ( board.canCastleQs<Color::white>() ) hash ^= castlingKeys[1];
            if ( board.canCastleKs<Color::black>() ) hash ^= castlingKeys[2];
            if ( board.canCastleQs<Color::black>() ) hash ^= castlingKeys[3];
        }

        // Update en passant
        if ( move.getFlag() == Move::Flag::pawn_push && abs(move.getFrom() - move.getTo()) == 16 ) {
            int ep_square = (board.whiteTurn()) ? move.getTo() + 8 : move.getTo() - 8;
            hash ^= enPassantKeys[ep_square];
        }

        return hash;
}