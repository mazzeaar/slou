#pragma once

#include <string>
#include <iostream>

#include "config.h"
#include "board/board.h"
#include "move.h"
#include "move_generator/move_generation.h"
#include "eval.h"

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
        board.move<color>(move);
        nodes += perft<utils::switchColor(color)>(board, depth - 1);
        board.undo<color>(move);
    }

    return nodes;
}

template <Color color>
u64 perft_entry(Board& board, int depth)
{
    MoveList move_list;
    u64 nodes = generate_moves<color>(move_list, board);

    if ( depth == 1 ) {
        return nodes;
    }

    nodes = 0ULL;
    for ( const auto& move : move_list ) {
        board.move<color>(move);
        u64 move_nodes = perft<utils::switchColor(color)>(board, depth - 1);
        board.undo<color>(move);

        std::cout << move.toLongAlgebraic() << ' ' << move_nodes << std::endl;
        nodes += move_nodes;
    }

    std::cout << '\n' << nodes << '\n';
    return nodes;
}

class CommandManager {
private:
    std::string _fen = STARTPOS;
    Board board = Board(_fen);
    std::string& to_lower(std::string& s) { for ( char c : s ) { c = std::tolower(c); } return s; }

    Move makeMoveFromString(const std::string& moveStr, const Board& board);

public:
    CommandManager() = default;
    void parseCommand();
};
