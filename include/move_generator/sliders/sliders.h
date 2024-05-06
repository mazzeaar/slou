#pragma once

#include "bitboard.h"
#include "definitions.h"
#include "magic/magic.h"

class sliders {
public:
    template <type::PieceType type, type::Color color>
    static void generateMoves(MoveList& move_list, const Board& board);

    template <type::PieceType type>
    static inline u64 getBitboard(u64 pieces, u64 occupancy);

private:
    template <type::PieceType type>
    static inline u64 getSquareMagic(u64 occupancy, int square);

    template <type::PieceType type>
    static inline u64 getPossibleMoves(u64 pieces, u64 occupancy);
};

#include "sliders_impl.hpp"