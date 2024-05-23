#pragma once

#include <string>
#include <vector>

#include "definitions.h"

#include "board/board.h"
#include "move.h"
#include "move_generator/move_generation.h"
#include "ttable.h"
class Game {
private:
    Board board;
    TranspositionTable ttable;

public:
    Game() = default;
    Game(const std::string& fen);

    void make_move(const std::string& algebraic_move);

    void unmake_move(const std::string& algebraic_move);

    uint64_t perftSimpleEntry(int depth);
    uint64_t perftDetailEntry(int depth);

private:
    Move moveFromSring(const std::string& algebraic_move);

    template <Color color, bool print_moves = false>
    uint64_t perft(Board& board, int depth);

    // for perftree
    template <Color color, bool print_moves = false>
    uint64_t debug_perft(Board& board, int depth);
};

template <Color color, bool print_moves>
uint64_t Game::perft(Board& board, int depth)
{
    MoveList list;
    uint64_t nodes = 0ULL;

    if ( ttable.lookup(board.getHash(), nodes) ) {
        return nodes;
    }

    generate_moves<color>(list, board);
    if ( depth == 1 ) {
        return list.size();
    }

    for ( const auto& move : list ) {
        board.move<color>(move);
        if constexpr ( print_moves ) {
            const uint64_t move_nodes = perft<utils::switchColor(color), false>(board, depth - 1);
            nodes += move_nodes;
            std::cout << move.toLongAlgebraic() << ' ' << move_nodes << '\n';
        }
        else {
            nodes += perft<utils::switchColor(color), false>(board, depth - 1);
        }
        board.undo<color>(move);
    }

    ttable.store(board.getHash(), nodes);

    return nodes;
}

template <Color color, bool print_moves>
uint64_t Game::debug_perft(Board& board, int depth)
{
    MoveList list;
    uint64_t nodes = 0ULL;

    generate_moves<color>(list, board);
    if ( depth == 0 ) {
        return 1ULL;
    }

    for ( const auto& move : list ) {
        board.move<color>(move);
        if constexpr ( print_moves ) {
            const uint64_t move_nodes = perft<utils::switchColor(color), false>(board, depth - 1);
            nodes += move_nodes;
            std::cout << move.toLongAlgebraic() << ' ' << move_nodes << '\n';
        }
        else {
            nodes += perft<utils::switchColor(color), false>(board, depth - 1);
        }
        board.undo<color>(move);
    }

    return nodes;
}