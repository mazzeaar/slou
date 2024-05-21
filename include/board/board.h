#pragma once

#include <string>
#include <vector>
#include <stack>

#include "definitions.h"
#include "bitboard.h"
#include "move.h"
#include "../zobrist.h"

// idea:

struct CompactState {
    // this will be a minimal state, can be used to restore unrecoverable information
};
struct BoardState {
    // state stuff like bitboards castling hmc etc

    // question: can the state be stored as an object:
    // State<color_to_play, has_ep_field, 4 x bool for castlingrights>();?
    // if yes: can i store such an oject in or on my board state and then pass the state to other functions?
    // this way i could do a lot at compiletime and i would have to check if i can caslte, because the template will have removed it already :D

    // this will need explicit instantiations of all comobinations:
    // so 2^6 combinations, for BoardState and for generateMoves as well, maybe other functions too
    CompactState storeState() const;

    // we restore the state using the previous move and the minimal previous state
    void restoreState(const CompactState& prev, const Move& move);

    // print functions can also be here, maybe templated castling rights?
};

struct MoveState {
    uint64_t ep_field_before = 0ULL;
    char castling_rights = 0x00;

    Piece moving_piece = Piece::none;
    Piece captured_piece = Piece::none;
    Piece promotion_piece = Piece::none;

    MoveState() = default;
};

class Board {
    Color cur_color = Color::white;
    std::array<uint64_t, 6 + 6 + 2> pieces = { 0ULL };
    uint64_t ep_field = 0ULL;

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

    template <Color color> void move(const Move& move);
    template <Color color> void undo(const Move& move);

    template <Color color>
    constexpr bool isCheck(uint64_t enemy_attacks) const { return (enemy_attacks & getPieces<PieceType::king, color>()) != NULL_BB; }

    /**
     * @brief Get the index of the piece board
     *
     * @param piece
     * @return constexpr int
     */
    constexpr int getIndex(Piece piece) const { return static_cast<int>(piece); }

    /**
     * @brief Get the index of the piece board
     *
     * @param type
     * @param color
     * @return constexpr int
     */
    constexpr int getIndex(PieceType type, Color color) const { return getIndex(utils::getPiece(type, color)); }

    /**
     * @brief   Get the index of the piece board
     *
     * @param type
     * @param color
     * @return constexpr int
     */
    template <PieceType type, Color color> constexpr static int getIndex();

    /**
     * @brief   Get the index of the piece board
     *
     * @param type
     * @param color
     * @return constexpr int
     */
    template <Piece piece> constexpr static int getIndex();

    /**
     * @brief   Get the piece on a specific square.
     *          Faster lookup is achieved by using a mailbox
     *
     * @param type
     * @param color
     * @return constexpr int
     */
    constexpr Piece getPiece(int square) const
    {
        return mailbox[square];
    }

    /**
     * @brief   Get the piece color on a specific square.
     *          Faster lookup is achieved by using a mailbox
     *
     * @param type
     * @param color
     * @return constexpr int
     */
    constexpr Color getColor(int square) const
    {
        return utils::pieceColor(getPiece(square));
    }

    /**
     * @brief   Get the type of piece on a specific square.
     *          Faster lookup is achieved by using a mailbox
     *
     * @param type
     * @param color
     * @return constexpr int
     */
    constexpr PieceType getPieceType(int square) const
    {
        return utils::getPieceType(getPiece(square));
    }

    /**
     * @brief Get the ep field bitboard
     *
     * @return constexpr uint64_t
     */
    constexpr uint64_t getEpField() const
    {
        return ep_field;
    }

    /**
     * @brief Get the bitboard of a specific piece
     *
     * @tparam type
     * @tparam color
     * @return constexpr uint64_t
     */
    template <PieceType type, Color color>
    constexpr uint64_t getPieces() const;

    /**
     * @brief Get the bitboard of a specific piece
     *
     * @tparam type
     * @tparam color
     * @return constexpr uint64_t
     */
    template <Piece piece>
    constexpr uint64_t getPieces() const;

    /**
     * @brief Get the occupancy (all pieces xor'ed)
     *
     * @return constexpr uint64_t
     */
    constexpr uint64_t getOccupancy() const
    {
        return pieces[12] | pieces[13];
    }

    /**
     * @brief Get the xored bitboard of all enemies
     *
     * @tparam color
     * @return constexpr uint64_t
     */
    template <Color color>
    constexpr uint64_t getEnemy() const
    {
        constexpr Color enemy_color = utils::switchColor(color);
        constexpr int enemy_idx = getIndex<PieceType::none, enemy_color>();
        return pieces[enemy_idx];
    }

    /**
     * @brief Can this color still castle?
     *
     * @param color
     * @return true
     * @return false
     */
    template <Color color>
    constexpr bool canCastle() const
    {
        return canCastleKs<color>() || canCastleQs<color>();
    }

    /**
     * @brief Can white or black still castle?
     *
     * @return true
     * @return false
     */
    constexpr bool canCastle() const
    {
        return canCastle<Color::white>() || canCastle<Color::black>();
    }

    /**
     * @brief Is color allowed to castle queenside?
     *
     * @param color
     * @return true
     * @return false
     */
    template <Color color>
    constexpr bool canCastleQs() const
    {
        if constexpr ( utils::isWhite(color) ) return castling_rights.white_qs != 0;
        else return castling_rights.black_qs != 0;
    }

    /**
     * @brief Is color allowed to castle kingside?
     *
     * @param color
     * @return true
     * @return false
     */
    template <Color color>
    constexpr bool canCastleKs() const
    {
        if constexpr ( utils::isWhite(color) ) return castling_rights.white_ks != 0;
        else return castling_rights.black_ks != 0;
    }

    /**
     * @brief Is it possible for color to castle kinside?
     *
     * @param color
     * @return true
     * @return false
     */
    template <Color color>
    constexpr bool canCastleKs(uint64_t enemy_attacks) const
    {
        if constexpr ( utils::isWhite(color) ) {
            return canCastleKs<color>() && ((getOccupancy() & 0x60) == 0) && ((enemy_attacks & 0x70) == 0);
        }
        else {
            return canCastleKs<color>() && ((getOccupancy() & (0x60ULL << 56)) == 0) && ((enemy_attacks & (0x70ULL << 56)) == 0);
        }
    }

    /**
     * @brief Is it possible for color to castle kinside?
     *
     * @param color
     * @return true
     * @return false
     */
    template <Color color>
    constexpr bool canCastleQs(uint64_t enemy_attacks) const
    {
        if constexpr ( utils::isWhite(color) ) {
            return canCastleQs<color>() && ((getOccupancy() & 0xE) == 0) && ((enemy_attacks & 0x1C) == 0);
        }
        else {
            return canCastleQs<color>() && ((getOccupancy() & (0xEULL << 56)) == 0) && ((enemy_attacks & (0x1CULL << 56)) == 0);
        }
    }

    template <Color color>
    constexpr void removeCastleKs()
    {
        if constexpr ( utils::isWhite(color) ) castling_rights.white_ks = 0;
        else castling_rights.black_ks = 0;
    }

    template <Color color>
    constexpr void removeCastleQs()
    {
        if constexpr ( utils::isWhite(color) ) castling_rights.white_qs = 0;
        else castling_rights.black_qs = 0;
    }

    template <Color color>
    constexpr void removeCastle()
    {
        removeCastleKs<color>();
        removeCastleQs<color>();
    }

    template <Color color>
    inline void remove_piece(Piece piece, int square);

    template <Color color>
    inline void place_piece(Piece piece, int square);

    template <Color color>
    inline void move_piece(Piece piece, int from, int to);

    // IMPORTANT! from & to are assumed to be the index of the piece, not the bitboard with the bit already set!
    template <PieceType type, Color color>
    constexpr void movePiece(uint64_t from, uint64_t to);

    // IMPORTANT! square is assumed to be the index of the piece, not the bitboard with the bit already set!
    template <PieceType type, Color color>
    constexpr void removePiece(uint64_t square);

    // IMPORTANT! square is assumed to be the index of the piece, not the bitboard with the bit already set!
    template <PieceType type, Color color>
    constexpr void placePiece(uint64_t square);

    std::string toString() const;
private:

    void storeState(const Move& move);
    void restoreState();

    constexpr void switchColor() { cur_color = utils::switchColor(cur_color); }

    template <Color color, bool is_capture>
    inline void tryToRemoveCastlingRights(const Move& move);
};

#include "board.hpp"