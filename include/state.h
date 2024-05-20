#pragma once

#include <iostream>
#include <cstdint>

#include "definitions.h"

class State {
    /** @brief white queenside castle not occupied */
    static constexpr uint64_t w_empty_q = 0b00001110ULL;
    /** @brief white kingside castle not occupied */
    static constexpr uint64_t w_empty_k = 0b01100000ULL;
    /** @brief white queenside castle not attacked */
    static constexpr uint64_t w_safe_q = 0b00011100ULL;
    /** @brief white kingside castle not attacked */
    static constexpr uint64_t w_safe_k = 0b01110000ULL;

    /** @brief black queenside castle not occupied */
    static constexpr uint64_t b_empty_q = (w_empty_q << 56ULL);
    /** @brief black kingside castle not occupied */
    static constexpr uint64_t b_empty_k = (w_empty_k << 56ULL);
    /** @brief black queenside castle not attacked */
    static constexpr uint64_t b_safe_q = (w_safe_q << 56ULL);
    /** @brief black kingside castle not attacked */
    static constexpr uint64_t b_safe_k = (w_safe_k << 56ULL);

    /** @brief white queenside rook position */
    static constexpr uint64_t w_rook_q = 0b00000001ULL;
    /** @brief white kingside rook position */
    static constexpr uint64_t w_rook_k = 0b10000000ULL;
    /** @brief black queenside rook position */
    static constexpr uint64_t b_rook_q = (w_rook_q << 56ULL);
    /** @brief black kingside rook position */
    static constexpr uint64_t b_rook_k = (w_rook_k << 56ULL);

    /** @brief white queenside rook castle move */
    static constexpr uint64_t w_rook_switch_q = 0b00001001ULL;
    /** @brief white kingsidse rook castle move */
    static constexpr uint64_t w_rook_switch_k = 0b10100000ULL;
    /** @brief white queenside king castle move */
    static constexpr uint64_t w_king_switch_q = 0b00010100ULL;
    /** @brief white kingside king castle move */
    static constexpr uint64_t w_king_switch_k = 0b00001010ULL;

    /** @brief black queenside rook castle move */
    static constexpr uint64_t b_rook_switch_q = (w_rook_switch_q << 56ULL);
    /** @brief black kingsidse rook castle move */
    static constexpr uint64_t b_rook_switch_k = (w_rook_switch_k << 56ULL);
    /** @brief black queenside king castle move */
    static constexpr uint64_t b_king_switch_q = (w_king_switch_q << 56ULL);
    /** @brief black kingside king castle move */
    static constexpr uint64_t b_king_switch_k = (w_king_switch_k << 56ULL);

public:
    const bool white_to_move;
    const bool has_ep_pawn;

    const bool w_castle_q;
    const bool w_castle_k;

    const bool b_castle_q;
    const bool b_castle_k;

    constexpr State(bool white_to_move, bool has_ep_pawn, bool w_castle_q, bool w_castle_k, bool b_castle_q, bool b_castle_k)
        : white_to_move(white_to_move), has_ep_pawn(has_ep_pawn),
        w_castle_q(w_castle_q), w_castle_k(w_castle_k),
        b_castle_q(b_castle_q), b_castle_k(b_castle_k)
    { }

    inline constexpr bool canCastle() const
    {
        if ( white_to_move ) return w_castle_q || w_castle_k;
        else return b_castle_q || b_castle_k;
    }

    inline constexpr bool canCastleQ() const
    {
        if ( white_to_move ) return w_castle_q;
        else return b_castle_q;
    }

    inline constexpr bool canCastleK() const
    {
        if ( white_to_move ) return w_castle_k;
        else return b_castle_k;
    }

    inline constexpr uint64_t getRookMoveQ() const
    {
        if ( white_to_move ) return w_rook_switch_q;
        else return b_rook_switch_q;
    }

    inline constexpr uint64_t getRookMoveK() const
    {
        if ( white_to_move ) return w_rook_switch_k;
        else return b_rook_switch_k;
    }

    inline constexpr uint64_t getKingMoveQ() const
    {
        if ( white_to_move ) return w_king_switch_q;
        else return b_king_switch_q;
    }

    inline constexpr uint64_t getKingMoveK() const
    {
        if ( white_to_move ) return w_king_switch_k;
        else return b_king_switch_k;
    }

    inline constexpr bool canCastleQ(uint64_t attacks, uint64_t occupancy) const
    {
        if ( white_to_move ) {
            return (occupancy & w_empty_q) || (attacks & w_safe_q);
        }
        else {
            return (occupancy & b_empty_q) || (attacks & b_safe_q);
        }
    }

    inline constexpr bool canCastleK(uint64_t attacks, uint64_t occupancy) const
    {
        if ( white_to_move ) {
            return (occupancy & w_empty_k) || (attacks & w_safe_k);
        }
        else {
            return (occupancy & b_empty_k) || (attacks & b_safe_k);
        }
    }

    /**
     * @brief Constexpr state transition for a pawn push
     *
     * @return constexpr State
     */
    inline constexpr State PawnPush() const
    {
        return State(!white_to_move, true, w_castle_q, w_castle_k, b_castle_q, b_castle_k);
    }

    /**
     * @brief Constexpr state transition for a king move
     *
     * @return constexpr State
     */
    inline constexpr State KingMove() const
    {
        if ( white_to_move ) {
            return State(!white_to_move, false, false, false, b_castle_q, b_castle_k);
        }
        else {
            return State(!white_to_move, false, w_castle_q, w_castle_k, false, false);
        }
    }

    /**
     * @brief Constexpr state transition for a kingside rook move
     *
     * @return constexpr State
     */
    inline constexpr State RookMoveK() const
    {
        if ( white_to_move ) {
            return State(!white_to_move, false, w_castle_q, false, b_castle_q, b_castle_k);
        }
        else {
            return State(!white_to_move, false, w_castle_q, w_castle_k, b_castle_q, false);
        }
    }

    /**
     * @brief Constexpr state transition for a queenside rook move
     *
     * @return constexpr State
     */
    inline constexpr State RookMoveQ() const
    {
        if ( white_to_move ) {
            return State(!white_to_move, false, false, w_castle_k, b_castle_q, b_castle_k);
        }
        else {
            return State(!white_to_move, false, w_castle_q, w_castle_k, false, b_castle_k);
        }
    }

    /**
     * @brief Constexpr state transition for a quiet move
     *
     * @return constexpr State
     */
    inline constexpr State QuietMove() const
    {
        return State(!white_to_move, false, w_castle_q, w_castle_k, b_castle_q, b_castle_k);
    }

    /**
     * @brief Constexpr default state for beginning a new game
     *
     * @return constexpr State
     */
    inline constexpr State Default() const
    {
        return State(true, false, true, true, true, true);
    }

    friend std::ostream& operator<<(std::ostream& os, const State& state)
    {
        if ( state.white_to_move ) os << 'w';
        else os << 'b';

        if ( state.has_ep_pawn ) os << " ep=true";
        else os << " ep=false";

        if ( !state.w_castle_k && !state.w_castle_q && !state.b_castle_k && !state.b_castle_q ) {
            os << " castle=false";
        }
        else {
            os << " castle=";
            if ( state.w_castle_q ) os << "Q";
            if ( state.w_castle_k ) os << "K";
            if ( state.b_castle_q ) os << "q";
            if ( state.b_castle_k ) os << "k";
        }

        return os;
    }
};