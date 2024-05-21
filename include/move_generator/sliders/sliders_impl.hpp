#pragma once

#include "sliders.h"

template <PieceType type, Color color>
void sliders::generateMoves(MoveList& move_list, const Board& board)
{
    static_assert(type == PieceType::bishop || type == PieceType::rook || type == PieceType::queen);

    const uint64_t occupancy = board.getOccupancy();
    const uint64_t enemy = board.getEnemy<color>();
    uint64_t pieces = board.getPieces<type, color>();

    BIT_LOOP(pieces)
    {
        const uint64_t from = get_LSB(pieces);
        const uint64_t potential_moves = getBitboard<type>((1ULL << from), occupancy);

        uint64_t attacks = potential_moves & enemy;
        BIT_LOOP(attacks)
        {
            const uint64_t to = get_LSB(attacks);
            move_list.add(Move::make<Move::Flag::capture>(from, to));
        }

        uint64_t moves = potential_moves & ~occupancy;
        BIT_LOOP(moves)
        {
            const uint64_t to = get_LSB(moves);
            move_list.add(Move::make<Move::Flag::quiet>(from, to));
        }
    }
}

template <PieceType type>
inline u64 sliders::getBitboard(u64 pieces, u64 occupancy)
{
    static_assert(type == PieceType::bishop || type == PieceType::rook || type == PieceType::queen);

    if constexpr ( utils::isQueen(type) ) {
        return getPossibleMoves<PieceType::bishop>(pieces, occupancy)
            | getPossibleMoves<PieceType::rook>(pieces, occupancy);
    }
    else {
        return getPossibleMoves<type>(pieces, occupancy);
    }
}

template <PieceType type>
inline u64 sliders::getSquareMagic(u64 occupancy, int square)
{
    occupancy &= magic::getMagics<type>(square).mask;
    occupancy *= magic::getMagics<type>(square).magic;
    occupancy >>= magic::getMagics<type>(square).shift;

    return magic::getMagics<type>(square).attack_table[occupancy];
}

template <PieceType type>
inline u64 sliders::getPossibleMoves(u64 pieces, u64 occupancy)
{
    u64 moves = 0ULL;
    BIT_LOOP(pieces)
    {
        const uint64_t from = get_LSB(pieces);
        moves |= getSquareMagic<type>(occupancy, from);
    }
    return moves;
}
