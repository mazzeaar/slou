#pragma once

#include <cstdint>
#include <cassert>
#include <string>
#include <iostream> // for cerr
#include <sstream> // for ostringstream

#include "definitions.h"

// https://www.chessprogramming.org/Encoding_Moves
struct Move final {
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

    static constexpr uint16_t CAPTURE_MASK = 0b0100;
    static constexpr uint16_t PROMO_MASK = 0b1000;

    static constexpr uint16_t RAW_CAPTURE_MASK = (0b0100 << FLAG_SHIFT);
    static constexpr uint16_t RAW_PROMO_MASK = (0b1000 << FLAG_SHIFT);

public:
    Move() : raw(0x0000) { }
    explicit constexpr Move(uint16_t raw) : raw(raw) { }

    Move(uint8_t from, uint8_t to, Flag flag)
        : raw(0x0000)
    {
        assert(from < 64 && to < 64); // checking bounds

        raw |= (static_cast<uint64_t>(flag) << FLAG_SHIFT);
        raw |= (from << FROM_SHIFT);
        raw |= (to << TO_SHIFT);
    }

    template <Flag flag>
    static constexpr inline Move make(uint8_t from, uint8_t to)
    {
        return Move((static_cast<uint64_t>(flag) << FLAG_SHIFT) | (from << FROM_SHIFT) | (to));
    }

    constexpr inline bool operator==(const Move& other) const { return other.raw == raw; }
    constexpr inline bool operator!=(const Move& other) const { return other.raw != raw; }

    constexpr inline uint8_t getFrom() const { return ((raw & FROM_MASK) >> FROM_SHIFT); }
    constexpr inline uint8_t getTo() const { return ((raw & TO_MASK) >> TO_SHIFT); }
    constexpr inline Flag getFlag() const { return static_cast<Flag>((raw & FLAG_MASK) >> FLAG_SHIFT); }

    constexpr inline bool hasQuietProperty() const { return static_cast<uint16_t>(getFlag()) <= static_cast<uint16_t>(Flag::ep); }

    constexpr inline bool isQuiet() const { return getFlag() == Flag::quiet; }
    constexpr inline bool isCapture() const { return raw & RAW_CAPTURE_MASK; }

    constexpr inline bool isEnpassant() const { return getFlag() == Flag::ep; }
    constexpr inline bool isDoublePawnPush() const { return getFlag() == Flag::pawn_push; }

    constexpr inline bool isCastle() const { return isKingCastle() || isQueenCastle(); }
    constexpr inline bool isKingCastle() const { return getFlag() == Flag::castle_k; }
    constexpr inline bool isQueenCastle() const { return getFlag() == Flag::castle_q; }

    constexpr inline bool isPromotion() const { return raw & RAW_PROMO_MASK; }
    constexpr inline bool isPromoCapture() const { return isCapture() && isPromotion(); }

    constexpr inline bool isKnightPromo() const { return (getFlag() == Flag::promo_n  || getFlag() == Flag::promo_x_n); }
    constexpr inline bool isBishopPromo() const { return (getFlag() == Flag::promo_b || getFlag() == Flag::promo_x_b); }
    constexpr inline bool isRookPromo() const { return (getFlag() == Flag::promo_r || getFlag() == Flag::promo_x_r); }
    constexpr inline bool isQueenPromo() const { return (getFlag() == Flag::promo_q || getFlag() == Flag::promo_x_q); }

    inline constexpr PieceType getPromotionPieceType() const
    {
        if ( isKnightPromo() ) return PieceType::knight;
        else if ( isBishopPromo() ) return PieceType::bishop;
        else if ( isRookPromo() ) return PieceType::rook;
        else if ( isQueenPromo() ) return PieceType::queen;
        else return PieceType::none;
    }

    std::string toLongAlgebraic() const;

private:
    inline std::string idxToNotation(unsigned idx) const { return utils::square_to_coordinates.at(idx); }
};

struct MoveList {
    std::array<Move, 256> moves = { Move() };   // max moves in a position is 218, this way we only need 512 bytes
    uint8_t count = 0;                          // enough for 256 moves

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