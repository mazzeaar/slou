#pragma once

#include <cstdint>
#include <cassert>
#include <string>
#include <iostream> // for cerr
#include <sstream> // for ostringstream

#include "definitions.h"

enum class MoveFlag : uint8_t {
    QUIET_MOVE = 0b0000,
    DOUBLE_PAWN_PUSH = 0b0001,
    KING_CASTLE = 0b0010,
    QUEEN_CASTLE = 0b0011,
    CAPTURE = 0b0100,
    EN_PASSANT = 0b0101,
    KNIGHT_PROMOTION = 0b1000,
    BISHOP_PROMOTION = 0b1001,
    ROOK_PROMOTION = 0b1010,
    QUEEN_PROMOTION = 0b1011,
    KNIGHT_PROMO_CAPTURE = 0b1100,
    BISHOP_PROMO_CAPTURE = 0b1101,
    ROOK_PROMO_CAPTURE = 0b1110,
    QUEEN_PROMO_CAPTURE = 0b1111
};

inline bool operator&(MoveFlag flag, uint16_t mask) { return static_cast<uint16_t>(flag) & mask; }
inline bool operator&(uint16_t mask, MoveFlag flag) { return mask & static_cast<uint16_t>(flag); }
inline uint16_t operator<<(MoveFlag flag, uint16_t shift) { return static_cast<uint16_t>(flag) << shift; }

// https://www.chessprogramming.org/Encoding_Moves
struct Move final {
private:
    uint16_t raw;

    /*
    MOVE = 0b 0000 0000 0000 0000 = 0x0000

    MASK
    FLAG = 0b 1111 0000 0000 0000 = 0xF000
    FROM = 0b 0000 1111 1100 0000 = 0x0FC0
    FROM = 0b 0000 0000 0011 1111 = 0x003F

    SHIFT
    FLAG = 0b 1111 0000 0000 0000 = 12
    FROM = 0b 0000 1111 1100 0000 = 6
    FROM = 0b 0000 0000 0011 1111 = 0
    */

    static constexpr uint16_t FLAG_MASK = 0xF000;
    static constexpr uint16_t FROM_MASK = 0x0FC0;
    static constexpr uint16_t TO_MASK = 0x003F;

    static constexpr uint16_t FLAG_SHIFT = 12;
    static constexpr uint16_t FROM_SHIFT = 6;
    static constexpr uint16_t TO_SHIFT = 0;

    static constexpr uint16_t CAPTURE_MASK = 0b0100;
    static constexpr uint16_t PROMO_MASK = 0b1000;

public:
    Move() : raw(0x0000) { }
    explicit constexpr Move(uint16_t raw) : raw(raw) { }
    Move(uint8_t from, uint8_t to, MoveFlag flag)
        : raw(0x0000)
    {
        assert(from < 64 && to < 64); // checking bounds

        raw |= (flag << FLAG_SHIFT);
        raw |= (from << FROM_SHIFT);
        raw |= (to << TO_SHIFT);
    }

    template <MoveFlag flag>
    static constexpr Move make(uint8_t from, uint8_t to)
    {
        return Move((flag << FLAG_SHIFT) | (from << FROM_SHIFT) | (to));
    }

    bool operator==(const Move& other) const { return other.raw == raw; }
    bool operator!=(const Move& other) const { return other.raw != raw; }

    constexpr inline uint8_t getFrom() const { return ((raw & FROM_MASK) >> FROM_SHIFT); }
    constexpr inline uint8_t getTo() const { return ((raw & TO_MASK) >> TO_SHIFT); }
    constexpr inline MoveFlag getFlag() const { return static_cast<MoveFlag>((raw & FLAG_MASK) >> FLAG_SHIFT); }

    constexpr inline bool hasQuietProperty() const { return static_cast<uint16_t>(getFlag()) <= static_cast<uint16_t>(MoveFlag::EN_PASSANT); }

    constexpr inline bool isQuiet() const { return getFlag() == MoveFlag::QUIET_MOVE; }
    constexpr inline bool isCapture() const { return getFlag() & CAPTURE_MASK; }

    constexpr inline bool isEnpassant() const { return getFlag() == MoveFlag::EN_PASSANT; }
    constexpr inline bool isDoublePawnPush() const { return getFlag() == MoveFlag::DOUBLE_PAWN_PUSH; }

    constexpr inline bool isCastle() const { return isKingCastle() || isQueenCastle(); }
    constexpr inline bool isKingCastle() const { return getFlag() == MoveFlag::KING_CASTLE; }
    constexpr inline bool isQueenCastle() const { return getFlag() == MoveFlag::QUEEN_CASTLE; }

    constexpr inline bool isPromotion() const { return getFlag() & PROMO_MASK; }
    constexpr inline bool isPromoCapture() const { return isCapture() && isPromotion(); }
    constexpr inline bool isKnightPromo() const { return (getFlag() == MoveFlag::KNIGHT_PROMOTION  || getFlag() == MoveFlag::KNIGHT_PROMO_CAPTURE); }
    constexpr inline bool isBishopPromo() const { return (getFlag() == MoveFlag::BISHOP_PROMOTION || getFlag() == MoveFlag::BISHOP_PROMO_CAPTURE); }
    constexpr inline bool isRookPromo() const { return (getFlag() == MoveFlag::ROOK_PROMOTION || getFlag() == MoveFlag::ROOK_PROMO_CAPTURE); }
    constexpr inline bool isQueenPromo() const { return (getFlag() == MoveFlag::QUEEN_PROMOTION || getFlag() == MoveFlag::QUEEN_PROMO_CAPTURE); }

    inline constexpr type::PieceType getPromotionPieceType() const
    {
        if ( isKnightPromo() ) return type::PieceType::knight;
        else if ( isBishopPromo() ) return type::PieceType::bishop;
        else if ( isRookPromo() ) return type::PieceType::rook;
        else if ( isQueenPromo() ) return type::PieceType::queen;
        else return type::PieceType::none;
    }

    std::string toLongAlgebraic() const;
    std::string toString() const;

private:
    inline std::string idxToNotation(unsigned idx) const { return type::square_to_coordinates.at(idx); }
};

struct MoveList {
    std::array<Move, 256> moves = { Move() };    // max moves in a position is 218, this way we only need 512 bytes
    uint8_t count = 0;              // enough for 256 moves

    inline void add(Move move)
    {
        if ( count < moves.size() ) {
            moves[count++] = move;
        }
    }

    void remove(size_t index)
    {
        if ( index < count ) {
            moves[index] = moves[--count];
        }
    }

    inline Move& operator[](size_t index) { return moves[index]; }
    inline const Move& operator[](size_t index) const { return moves[index]; }

    inline size_t size() const { return count; }
    inline void clear() { count = 0; }

    // iterators for range based loops
    inline Move* begin() { return moves.data(); }
    inline Move* end() { return moves.data() + count; }
    inline const Move* begin() const { return moves.data(); }
    inline const Move* end() const { return moves.data() + count; }
};