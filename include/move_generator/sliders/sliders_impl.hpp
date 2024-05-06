#pragma once

#include "sliders.h"

template <type::PieceType type, type::Color color>
void sliders::generateMoves(MoveList& move_list, const Board& board)
{
    static_assert(type == type::PieceType::bishop || type == type::PieceType::rook || type == type::PieceType::queen);

    DEBUG_START;

    const u64 occupancy = board.getOccupancy();
    const u64 enemy = board.getEnemy(color);
    u64 pieces = board.getBoard(type, color);

    while ( pieces ) {
        u64 origin_bit = pop_lsb_to_u64(pieces);
        const uint8_t from = get_LSB(origin_bit);

        u64 potential_moves = getBitboard<type>(origin_bit, occupancy);
        while ( potential_moves ) {
            u64 target_bit = pop_lsb_to_u64(potential_moves);
            uint8_t to = get_LSB(target_bit);

            if ( target_bit & enemy ) { // eat
                move_list.add(Move(from, to, MoveFlag::CAPTURE));
            }
            else if ( target_bit & ~occupancy ) { // move
                move_list.add(Move(from, to, MoveFlag::QUIET_MOVE));
            } // else we are blocked
        }
    }

    DEBUG_END;
}

template <type::PieceType type>
inline u64 sliders::getBitboard(u64 pieces, u64 occupancy)
{
    static_assert(type == type::PieceType::bishop || type == type::PieceType::rook || type == type::PieceType::queen);

    if constexpr ( type::isQueen(type) ) {
        return getPossibleMoves<type::PieceType::bishop>(pieces, occupancy)
            | getPossibleMoves<type::PieceType::rook>(pieces, occupancy);
    }
    else {
        return getPossibleMoves<type>(pieces, occupancy);
    }
}

template <type::PieceType type>
inline u64 sliders::getSquareMagic(u64 occupancy, int square)
{
    occupancy &= magic::getMagics<type>(square).mask;
    occupancy *= magic::getMagics<type>(square).magic;
    occupancy >>= magic::getMagics<type>(square).shift;

    return magic::getMagics<type>(square).attack_table[occupancy];
}

template <type::PieceType type>
inline u64 sliders::getPossibleMoves(u64 pieces, u64 occupancy)
{
    u64 moves = 0ULL;
    while ( pieces ) {
        u64 origin_bit = extract_next_bit(pieces);
        const int idx = get_LSB(origin_bit);
        moves |= getSquareMagic<type>(occupancy, idx);
    }
    return moves;
}
