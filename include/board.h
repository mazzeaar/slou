#pragma once

#include <string>
#include <vector>
#include <stack>

#include "definitions.h"
#include "bitboard.h"
#include "move.h"
#include "debug.h"

struct MoveState {
    u64 ep_field_before = 0ULL;
    char castling_rights = 0x00;

    type::Piece moving_piece = type::Piece::none;
    type::Piece captured_piece = type::Piece::none;
    type::Piece promotion_piece = type::Piece::none;

    MoveState() = default;
};

struct FullState {
    type::Color s_color;
    u64 s_pieces[12];
    u64 s_ep_field;
    char s_castling_rights;

    type::Piece moving_piece = type::Piece::none;
    type::Piece captured_piece = type::Piece::none;
    type::Piece promotion_piece = type::Piece::none;
};

class Board {
    static constexpr int size = 12;
    type::Color cur_color = type::Color::white;
    u64 pieces[size] = { 0ULL };
    u64 ep_field = 0ULL;

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
    // std::stack<FullState> move_history;
public:

    Board(const std::string& fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    std::string getFen() const;

    inline bool whiteTurn() const { return type::isWhite(cur_color); }

    u64 getEpField() const
    {
        return ep_field;
    }

    void move(const Move& move);
    void undo(const Move& move);

    std::string toString() const;
    std::string toPrettyString(bool colored = false, bool emoji = true) const;

private:

    constexpr void switchColor() { cur_color = type::switchColor(cur_color); }

    void updateEpField(u64 new_ep_field) { ep_field = new_ep_field; }

    void tryToRemoveCastlingRights(const Move& move);

    std::vector<std::vector<char>> toCharMailbox() const;
    std::vector<std::vector<std::string>> toStringMailbox(bool emoji = false) const;

    // ALREADY REFACTORED
public:
    constexpr inline int getIndex(type::Piece piece) const { return static_cast<int>(piece); }
    constexpr inline int getIndex(type::PieceType type, type::Color color) const { return getIndex(type::getPiece(type, color)); }

    constexpr inline type::Piece getPieceFromSquare(int square) const
    {
        const u64 s = single_bit_u64(square);
        for ( int i = 0; i < 12; ++i ) {
            if ( (pieces[i] & s) != 0ULL ) {
                return static_cast<type::Piece>(i);
            }
        }
        return type::Piece::none;
    }

    constexpr inline type::Color getColorFromSquare(int square) const { return type::pieceColor(getPieceFromSquare(square)); }
    constexpr inline type::PieceType getPieceTypeFromSquare(int square) const { return type::getPieceType(getPieceFromSquare(square)); }

    constexpr inline u64 getOccupancy() const
    {
        return pieces[0] | pieces[1] | pieces[2] | pieces[3] | pieces[4] | pieces[5]
            | pieces[6] | pieces[7] | pieces[8] | pieces[9] | pieces[10] | pieces[11];
    }

    constexpr inline u64 getEnemy(type::Color color) const
    {
        type::Color enemy_color = type::switchColor(color);
        return getPawns(enemy_color) | getKnights(enemy_color) | getBishops(enemy_color)
            | getRooks(enemy_color) | getQueens(enemy_color) | getKing(enemy_color);
    }

    constexpr inline u64 getBoard(type::Piece piece) const { return pieces[getIndex(piece)]; }
    constexpr inline u64 getBoard(type::PieceType type, type::Color color) const { return pieces[getIndex(type, color)]; }

    // ================================
    // Get Specific Pieces
    // ================================

    constexpr inline u64 getPawns(type::Color color) const { return getBoard(type::PieceType::pawn, color); }
    constexpr inline u64 getKnights(type::Color color) const { return getBoard(type::PieceType::knight, color); }
    constexpr inline u64 getBishops(type::Color color) const { return getBoard(type::PieceType::bishop, color); }
    constexpr inline u64 getRooks(type::Color color) const { return getBoard(type::PieceType::rook, color); }
    constexpr inline u64 getQueens(type::Color color) const { return getBoard(type::PieceType::queen, color); }
    constexpr inline u64 getKing(type::Color color) const { return getBoard(type::PieceType::king, color); }

    // ================================
    // Castling Rights
    // ================================
    constexpr inline bool canCastleQs(type::Color color) const
    {
        if ( type::isWhite(color) ) return castling_rights.white_qs != 0;
        else return castling_rights.black_qs != 0;
    }

    constexpr inline bool canCastleKs(type::Color color) const
    {
        if ( type::isWhite(color) ) return castling_rights.white_ks != 0;
        else return castling_rights.black_ks != 0;
    }

    constexpr inline bool canCastleKs(type::Color color, u64 enemy_attacks) const
    {
        if ( type::isWhite(color) ) {
            return canCastleKs(color) && ((getOccupancy() & 0x60) == 0) && ((enemy_attacks & 0x70) == 0);
        }
        else {
            return canCastleKs(color) && ((getOccupancy() & (0x60ULL << 56)) == 0) && ((enemy_attacks & (0x70ULL << 56)) == 0);
        }
    }

    constexpr inline bool canCastleQs(type::Color color, u64 enemy_attacks) const
    {
        if ( type::isWhite(color) ) {
            return canCastleQs(color) && ((getOccupancy() & 0xE) == 0) && ((enemy_attacks & 0x1C) == 0);
        }
        else {
            return canCastleQs(color) && ((getOccupancy() & (0xEULL << 56)) == 0) && ((enemy_attacks & (0x1CULL << 56)) == 0);
        }
    }

    // 0 0 0 0 0 0 0 0

    constexpr inline bool canCastle(type::Color color) const { return canCastleKs(color) || canCastleQs(color); }
    constexpr inline bool canCastle() const { return canCastle(type::Color::white) || canCastle(type::Color::black); }

    constexpr inline void removeCastleKs(type::Color color)
    {
        if ( type::isWhite(color) ) castling_rights.white_ks = 0;
        else castling_rights.black_ks = 0;
    }

    constexpr inline void removeCastleQs(type::Color color)
    {
        if ( type::isWhite(color) ) castling_rights.white_qs = 0;
        else castling_rights.black_qs = 0;
    }

    constexpr inline void removeCastle(type::Color color)
    {
        removeCastleKs(color);
        removeCastleQs(color);
    }

    // ================================
    // Place pieces
    // ================================

    inline void setPiece(type::Piece piece, int square)
    {
        assert(0 <= square && square < 64 && "square for setPiece is out of bounds");
        pieces[getIndex(piece)] |= single_bit_u64(square);
    }

    inline void setPiece(type::PieceType type, type::Color color, int square) { setPiece(type::getPiece(type, color), square); }

    inline void remove_piece(type::Piece piece, int square) { pieces[getIndex(piece)] ^= single_bit_u64(square); }
    inline void place_piece(type::Piece piece, int square) { pieces[getIndex(piece)] |= single_bit_u64(square); }
    inline void move_piece(type::Piece piece, int from, int to) { pieces[getIndex(piece)] ^= (single_bit_u64(from) | single_bit_u64(to)); }

    // ================================
    // Stuff
    // ================================

    inline bool isCheck(type::Color color, u64 enemy_attacks) const
    {
        return (enemy_attacks & getBoard(type::PieceType::king, color)) != 0ULL;
    }

    void storeState(const Move& move);
    void restoreState();
};
