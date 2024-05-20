#pragma once

#include <string>
#include <vector>
#include <stack>

#include "definitions.h"
#include "bitboard.h"
#include "move.h"
#include "debug.h"
#include "../zobrist.h"

struct MoveState {
    u64 ep_field_before = 0ULL;
    char castling_rights = 0x00;

    Piece moving_piece = Piece::none;
    Piece captured_piece = Piece::none;
    Piece promotion_piece = Piece::none;

    MoveState() = default;
};

class Board {
    Color cur_color = Color::white;
    std::array<uint64_t, 6 + 6 + 2> pieces = { 0ULL };
    u64 ep_field = 0ULL;

    std::array<Piece, 64> mailbox { Piece::none };

    union {
        struct {
            bool white_qs : 1;
            bool white_ks : 1;
            bool black_qs : 1;
            bool black_ks : 1;
        };
        char raw = 0xFF;
    } castling_rights;

    int half_move_clock = 0;
    int full_move_clock = 1;

    std::stack<MoveState> move_history;
public:

    Board(const std::string& fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    std::string getFen() const;

    inline bool whiteTurn() const { return utils::isWhite(cur_color); }

    constexpr inline u64 getEpField() const { return ep_field; }

    template <Color color> void move(const Move& move);
    template <Color color> void undo(const Move& move);

    std::string toString() const;
    std::string toPrettyString(bool colored = false, bool emoji = true) const;

private:
    constexpr void switchColor() { cur_color = utils::switchColor(cur_color); }

    template <Color color, bool is_capture>
    inline void tryToRemoveCastlingRights(const Move& move);

    std::vector<std::vector<char>> toCharMailbox() const;
    std::vector<std::vector<std::string>> toStringMailbox(bool emoji = false) const;

    // ALREADY REFACTORED
public:
    constexpr inline int getPieceIndex(Piece piece) const { return static_cast<int>(piece); }
    constexpr inline int getPieceIndex(PieceType type, Color color) const { return getPieceIndex(utils::getPiece(type, color)); }

    template <PieceType type, Color color> static constexpr inline int getIndex();
    template <Piece piece> static constexpr inline int getIndex();

    constexpr inline Piece getPieceFromSquare(int square) const { return mailbox[square]; }
    constexpr inline Color getColorFromSquare(int square) const { return utils::pieceColor(getPieceFromSquare(square)); }
    constexpr inline PieceType getPieceTypeFromSquare(int square) const { return utils::getPieceType(getPieceFromSquare(square)); }

    template <PieceType type, Color color> constexpr inline uint64_t getPieces() const;
    template <Piece piece> constexpr inline uint64_t getPieces() const;
    constexpr inline u64 getOccupancy() const { return pieces[12] | pieces[13]; }

    template <Color color>
    constexpr inline u64 getEnemy() const
    {
        constexpr Color enemy_color = utils::switchColor(color);
        constexpr int enemy_idx = getIndex<PieceType::none, enemy_color>();
        return pieces[enemy_idx];
    }

    template <PieceType type, Color color>
    constexpr inline u64 getBoard() const
    {
        constexpr int index = getIndex<type, color>();
        return pieces[index];
    }

    // ================================
    // Castling Rights
    // ================================
    constexpr inline bool canCastleQs(Color color) const
    {
        if ( utils::isWhite(color) ) return castling_rights.white_qs != 0;
        else return castling_rights.black_qs != 0;
    }

    constexpr inline bool canCastleKs(Color color) const
    {
        if ( utils::isWhite(color) ) return castling_rights.white_ks != 0;
        else return castling_rights.black_ks != 0;
    }

    constexpr inline bool canCastleKs(Color color, u64 enemy_attacks) const
    {
        if ( utils::isWhite(color) ) {
            return canCastleKs(color) && ((getOccupancy() & 0x60) == 0) && ((enemy_attacks & 0x70) == 0);
        }
        else {
            return canCastleKs(color) && ((getOccupancy() & (0x60ULL << 56)) == 0) && ((enemy_attacks & (0x70ULL << 56)) == 0);
        }
    }

    constexpr inline bool canCastleQs(Color color, u64 enemy_attacks) const
    {
        if ( utils::isWhite(color) ) {
            return canCastleQs(color) && ((getOccupancy() & 0xE) == 0) && ((enemy_attacks & 0x1C) == 0);
        }
        else {
            return canCastleQs(color) && ((getOccupancy() & (0xEULL << 56)) == 0) && ((enemy_attacks & (0x1CULL << 56)) == 0);
        }
    }

    // 0 0 0 0 0 0 0 0

    constexpr inline bool canCastle(Color color) const { return canCastleKs(color) || canCastleQs(color); }
    constexpr inline bool canCastle() const { return canCastle(Color::white) || canCastle(Color::black); }

    constexpr inline void removeCastleKs(Color color)
    {
        if ( utils::isWhite(color) ) castling_rights.white_ks = 0;
        else castling_rights.black_ks = 0;
    }

    constexpr inline void removeCastleQs(Color color)
    {
        if ( utils::isWhite(color) ) castling_rights.white_qs = 0;
        else castling_rights.black_qs = 0;
    }

    constexpr inline void removeCastle(Color color)
    {
        removeCastleKs(color);
        removeCastleQs(color);
    }

    // ================================
    // Place pieces
    // ================================

    template <Color color>
    inline void remove_piece(Piece piece, int square);

    template <Color color>
    inline void place_piece(Piece piece, int square);

    template <Color color>
    inline void move_piece(Piece piece, int from, int to);

    // IMPORTANT! from & to are assumed to be the index of the piece, not the bitboard with the bit already set!
    template <PieceType type, Color color>
    constexpr inline void movePiece(uint64_t from, uint64_t to);

    // IMPORTANT! square is assumed to be the index of the piece, not the bitboard with the bit already set!
    template <PieceType type, Color color>
    constexpr inline void removePiece(uint64_t square);

    // IMPORTANT! square is assumed to be the index of the piece, not the bitboard with the bit already set!
    template <PieceType type, Color color>
    constexpr inline void placePiece(uint64_t square);

    // ================================
    // Stuff
    // ================================

    template <Color color>
    constexpr inline bool isCheck(u64 enemy_attacks) const
    {
        return (enemy_attacks & getBoard<PieceType::king, color>()) != 0ULL;
    }

    void storeState(const Move& move);
    void restoreState();
};

#include "board.hpp"