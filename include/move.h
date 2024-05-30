#pragma once

#include <cassert>
#include <string>

#include "definitions.h"

// https://www.chessprogramming.org/Encoding_Moves
struct Move {
public:
    enum class Flag : uint8_t {
        quiet = 0b0000,
        pawn_push = 0b0001,
        castle_k = 0b0010,
        castle_q = 0b0011,
        capture = 0b0100,
        ep = 0b0101,
        promo_n = 0b1000,
        promo_b = 0b1001,
        promo_r = 0b1010,
        promo_q = 0b1011,
        promo_x_n = 0b1100,
        promo_x_b = 0b1101,
        promo_x_r = 0b1110,
        promo_x_q = 0b1111
    };

private:
    uint16_t raw;

    static constexpr uint16_t FLAG_MASK = 0xF000;
    static constexpr uint16_t FROM_MASK = 0x0FC0;
    static constexpr uint16_t TO_MASK = 0x003F;

    static constexpr uint16_t FLAG_SHIFT = 12;
    static constexpr uint16_t FROM_SHIFT = 6;
    static constexpr uint16_t TO_SHIFT = 0;

    static constexpr uint16_t RAW_CAPTURE_MASK = (0b0100 << FLAG_SHIFT);
    static constexpr uint16_t RAW_PROMO_MASK = (0b1000 << FLAG_SHIFT);

public:
    constexpr Move() : raw(0x0000) { }
    explicit constexpr Move(uint16_t raw) : raw(raw) { }

    constexpr Move(uint8_t from, uint8_t to, Flag flag)
        : raw((static_cast<uint16_t>(flag) << FLAG_SHIFT) | (from << FROM_SHIFT) | (to << TO_SHIFT))
    {
        // assert(from < 64 && to < 64); // checking bounds
    }

    template <Flag flag>
    static constexpr Move make(uint8_t from, uint8_t to)
    {
        return Move((static_cast<uint16_t>(flag) << FLAG_SHIFT) | (from << FROM_SHIFT) | (to << TO_SHIFT));
    }

    constexpr bool operator==(const Move& other) const { return raw == other.raw; }
    constexpr bool operator!=(const Move& other) const { return raw != other.raw; }

    constexpr uint8_t getFrom() const { return (raw & FROM_MASK) >> FROM_SHIFT; }
    constexpr uint8_t getTo() const { return (raw & TO_MASK) >> TO_SHIFT; }
    constexpr Flag getFlag() const { return static_cast<Flag>((raw & FLAG_MASK) >> FLAG_SHIFT); }

    constexpr bool isQuiet() const { return getFlag() == Flag::quiet; }
    constexpr bool isCapture() const { return raw & RAW_CAPTURE_MASK; }
    constexpr bool isEnpassant() const { return getFlag() == Flag::ep; }
    constexpr bool isDoublePawnPush() const { return getFlag() == Flag::pawn_push; }
    constexpr bool isCastle() const { return isKingCastle() || isQueenCastle(); }
    constexpr bool isKingCastle() const { return getFlag() == Flag::castle_k; }
    constexpr bool isQueenCastle() const { return getFlag() == Flag::castle_q; }
    constexpr bool isPromotion() const { return raw & RAW_PROMO_MASK; }
    constexpr bool isPromoCapture() const { return isCapture() && isPromotion(); }
    constexpr bool isKnightPromo() const { return getFlag() == Flag::promo_n || getFlag() == Flag::promo_x_n; }
    constexpr bool isBishopPromo() const { return getFlag() == Flag::promo_b || getFlag() == Flag::promo_x_b; }
    constexpr bool isRookPromo() const { return getFlag() == Flag::promo_r || getFlag() == Flag::promo_x_r; }
    constexpr bool isQueenPromo() const { return getFlag() == Flag::promo_q || getFlag() == Flag::promo_x_q; }

    template <Color color>
    constexpr Piece getPromotionPiece() const;
    constexpr PieceType getPromotionPieceType() const;

    inline std::string toLongAlgebraic() const;

private:
    inline std::string_view idxToNotation(unsigned idx) const { return utils::square_to_coordinates.at(idx); }
};

constexpr PieceType Move::getPromotionPieceType() const
{
    const Flag flag = getFlag();
    switch ( flag ) {
        case Flag::promo_n:
        case Flag::promo_x_n:
            return PieceType::knight;
        case Flag::promo_b:
        case Flag::promo_x_b:
            return PieceType::bishop;
        case Flag::promo_r:
        case Flag::promo_x_r:
            return PieceType::rook;
        case Flag::promo_q:
        case Flag::promo_x_q:
            return PieceType::queen;
        default:
            return PieceType::none;
    }
}

template <Color color>
constexpr Piece Move::getPromotionPiece() const
{
    const Flag flag = getFlag();
    switch ( flag ) {
        case Flag::promo_n:
        case Flag::promo_x_n:
            return utils::getPiece(PieceType::knight, color);
        case Flag::promo_b:
        case Flag::promo_x_b:
            return utils::getPiece(PieceType::bishop, color);
        case Flag::promo_r:
        case Flag::promo_x_r:
            return utils::getPiece(PieceType::rook, color);
        case Flag::promo_q:
        case Flag::promo_x_q:
            return utils::getPiece(PieceType::queen, color);
        default:
            return Piece::none;
    }
}

inline std::string Move::toLongAlgebraic() const
{
    std::string moveStr = "";
    moveStr += idxToNotation(getFrom());
    moveStr += idxToNotation(getTo());

    const Flag flag = getFlag();
    switch ( flag ) {
        case Flag::promo_n:
        case Flag::promo_x_n: {
            moveStr += "n";
        } break;
        case Flag::promo_b:
        case Flag::promo_x_b: {
            moveStr += "b";
        } break;
        case Flag::promo_r:
        case Flag::promo_x_r: {
            moveStr += "r";
        } break;
        case Flag::promo_q:
        case Flag::promo_x_q: {
            moveStr += "q";
        } break;
        default: break;
    }

    return moveStr;
}

struct MoveList {
    std::array<Move, 256> moves = { Move() };   // max moves in a position is 218, this way we only need 512 bytes
    uint8_t count = 0;                          // enough for 256 moves

    constexpr void add(Move move)
    {
        if ( count < moves.size() ) {
            moves[count++] = move;
        }
    }

    constexpr void remove(size_t index)
    {
        if ( index < count ) {
            moves[index] = moves[--count];
        }
    }

    constexpr Move& operator[](size_t index) { return moves[index]; }
    constexpr const Move& operator[](size_t index) const { return moves[index]; }

    constexpr size_t size() const { return count; }
    constexpr void clear() { count = 0; }

    // for range based for loops
    constexpr Move* begin() { return moves.data(); }
    constexpr Move* end() { return moves.data() + count; }
    constexpr const Move* begin() const { return moves.data(); }
    constexpr const Move* end() const { return moves.data() + count; }
};
