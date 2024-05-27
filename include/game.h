#pragma once

#include <string>
#include <vector>

#include "definitions.h"

#include "board/board.h"
#include "move.h"
#include "move_generator/move_generation.h"
#include "ttable.h"
#include "eval.h"
#include "config.h"

class Game {
private:
    Board board;
    TTable<TTEntry_perft, TTABLE_SIZE_MB> tt_perft;

public:
    Game() = default;
    Game(const std::string& fen);

    void make_move(const std::string& algebraic_move);
    void unmake_move(const std::string& algebraic_move);

    Move bestMove(int depth = 5);

    uint64_t perftSimpleEntry(int depth);
    uint64_t perftDetailEntry(int depth);

    std::string toString() const { return board.toString(); }

    template <Color color>
    Move getBestMove(Board& board, int depth = 5);

private:
    Move moveFromSring(const std::string& algebraic_move);

    template <Color color, bool print_moves = false>
    uint64_t perft(Board& board, int depth);

    // for perftree
    template <Color color, bool print_moves = false>
    uint64_t debug_perft(Board& board, int depth);

    template <Color color>
    double minimax(Board& board, int depth, double alpha, double beta);
};

template <Color color, bool print_moves>
uint64_t Game::perft(Board& board, int depth)
{
    uint64_t nodes = 0ULL;
    uint64_t key = board.getZobristKey();
    if ( tt_perft.if_has_get(key, depth, nodes) ) {
        return nodes;
    }

    MoveList list;

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

    tt_perft.emplace(key, nodes, depth);
    return nodes;
}

template <Color color, bool print_moves>
uint64_t Game::debug_perft(Board& board, int depth)
{
    uint64_t nodes = 0ULL;
    uint64_t key = board.getZobristKey();
    if ( tt_perft.if_has_get(key, depth, nodes) ) {
        return nodes;
    }

    MoveList list;

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

    tt_perft.emplace(key, nodes, depth);
    return nodes;
}

template <Color color>
Move Game::getBestMove(Board& board, int depth)
{
    /*
    uint64_t key = board.getZobristKey();
    if ( tt_eval.has(key, depth) ) {
        auto entry = tt_eval.get(key, depth);
        return entry->best_move;
    }
    */

    MoveList move_list;
    generate_moves<color>(move_list, board);

    Move best_move;
    double best_score = (color == Color::white) ? -INFTY : INFTY;
    double alpha = -INFTY;
    double beta = INFTY;

    for ( const auto& move : move_list ) {
        board.move<color>(move);
        double score = minimax<utils::switchColor(color)>(board, depth - 1, alpha, beta);
        board.undo<color>(move);

        if ( (color == Color::white && score > best_score) || (color == Color::black && score < best_score) ) {
            best_score = score;
            best_move = move;
        }
    }

    // tt_eval.addEntry(key, depth, best_move, best_score, TTEntry_eval::EXACT);

    return best_move;
}

template <Color color>
double Game::minimax(Board& board, int depth, double alpha, double beta)
{
    /*
    uint64_t key = board.getZobristKey();
    if ( tt_eval.has(key, depth) ) {
        auto entry = tt_eval.get(key, depth);
        return entry->score;
    }
    */

    if ( depth == 0 ) {
        return evalPosition<color>(board);
    }

    MoveList move_list;
    generate_moves<color>(move_list, board);

    // no moves -> checkmate or stalemate
    if ( move_list.size() == 0 ) {
        const uint64_t enemy_attacks = generate_attacks<utils::switchColor(color)>(board);
        if ( board.isCheck<color>(enemy_attacks) ) {
            return (utils::isWhite(color)) ? -INFTY : INFTY;
        }
        else {
            return 0;
        }
    }

    double best_score = (utils::isWhite(color)) ? -INFTY : INFTY;
    Move best_move;
    for ( const auto& move : move_list ) {
        board.move<color>(move);
        double score = minimax<utils::switchColor(color)>(board, depth - 1, alpha, beta);
        board.undo<color>(move);

        if constexpr ( color == Color::white ) {
            if ( score > best_score ) {
                best_score = score;
                best_move = move;
            }

            alpha = std::max(alpha, score);
        }
        else {
            if ( score < best_score ) {
                best_score = score;
                best_move = move;
            }

            beta = std::min(beta, score);
        }


        if ( beta <= alpha ) {
            break; // pruning
        }
    }

    /*
    auto type = TTEntry_eval::EXACT;
    if ( best_score <= alpha ) {
        type = TTEntry_eval::UPPERBOUND;
    }
    else if ( best_score >= beta ) {
        type = TTEntry_eval::LOWERBOUND;
    }
    tt_eval.addEntry(key, depth, best_move, best_score, type);
    */

    return best_score;
}