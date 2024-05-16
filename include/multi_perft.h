#pragma once

#include "definitions.h"
#include "move_generator/move_generation.h"
#include "board/board.h"
#include "move.h"
#include "zobrist.h"
#include "ttable.h"

template <Color color>
u64 perft(Board& board, int depth)
{
    MoveList move_list;
    u64 nodes = generate_moves<color>(move_list, board);

    if ( depth == 1 ) {
        return nodes;
    }

    nodes = 0ULL;
    for ( const auto& move : move_list ) {
        board.move(move);
        nodes += perft<utils::switchColor(color)>(board, depth - 1);
        board.undo(move);
    }

    return nodes;
}