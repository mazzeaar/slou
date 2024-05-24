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
    uint64_t updateHash(uint64_t hash, const Move& move, const Board& board);

    inline void togglePiece(uint64_t& hash, int piece_id, int square);
    inline void toggleCastlingRights(uint64_t& hash, const Board& board);
    inline void toggleEnPassant(uint64_t& hash, uint64_t ep_field);
    inline void toggleBlackToMove(uint64_t& hash);
    inline void updatePieceMove(uint64_t& hash, const Move& move, const Board& board);
    inline void updateCapture(uint64_t& hash, const Move& move, const Board& board);
    inline void updateCastling(uint64_t& hash, const Move& move, const Board& board);
    inline void updateEnPassantCapture(uint64_t& hash, const Move& move, const Board& board);
    inline void updatePromotion(uint64_t& hash, const Move& move, const Board& board);
    inline void updateEnPassantField(uint64_t& hash, const Move& move, const Board& board);


    void togglePiece(uint64_t& hash, int piece_id, int square)
    {
        hash ^= pieceKeys[piece_id][square];
    }

    void toggleCastlingRights(uint64_t& hash, const Board& board)
    {
        if ( board.canCastleKs<Color::white>() ) hash ^= castlingKeys[0];
        if ( board.canCastleQs<Color::white>() ) hash ^= castlingKeys[1];
        if ( board.canCastleKs<Color::black>() ) hash ^= castlingKeys[2];
        if ( board.canCastleQs<Color::black>() ) hash ^= castlingKeys[3];
    }

    void toggleEnPassant(uint64_t& hash, uint64_t ep_field)
    {
        if ( ep_field != 0ULL ) {
            hash ^= enPassantKeys[get_LSB(ep_field)];
        }
    }

    void toggleBlackToMove(uint64_t& hash)
    {
        hash ^= blackToMove;
    }

    void updatePieceMove(uint64_t& hash, const Move& move, const Board& board)
    {
        int piece_id = board.getIndex(board.getPiece(move.getFrom()));
        togglePiece(hash, piece_id, move.getFrom());
        togglePiece(hash, piece_id, move.getTo());
    }

    void updateCapture(uint64_t& hash, const Move& move, const Board& board)
    {
        int captured_piece_id = board.getIndex(board.getPiece(move.getTo()));
        togglePiece(hash, captured_piece_id, move.getTo());
    }

    void updateCastling(uint64_t& hash, const Move& move, const Board& board)
    {
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
        togglePiece(hash, rook_id, rook_from);
        togglePiece(hash, rook_id, rook_to);
    }

    void updateEnPassantCapture(uint64_t& hash, const Move& move, const Board& board)
    {
        int ep_square = (board.whiteTurn()) ? move.getTo() - 8 : move.getTo() + 8;
        int captured_piece_id = board.getIndex(board.getPiece(ep_square));
        togglePiece(hash, captured_piece_id, ep_square);
    }

    void updatePromotion(uint64_t& hash, const Move& move, const Board& board)
    {
        int promo_id = board.getIndex(utils::getPiece(move.getPromotionPieceType(), board.whiteTurn() ? Color::white : Color::black));
        togglePiece(hash, promo_id, move.getTo());
    }

    void updateEnPassantField(uint64_t& hash, const Move& move, const Board& board)
    {
        int ep_square = (board.whiteTurn()) ? move.getTo() + 8 : move.getTo() - 8;
        hash ^= enPassantKeys[ep_square];
    }
};
