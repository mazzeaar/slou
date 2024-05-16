#pragma once

#include "bitboard.h"
#include "board/board.h"
#include "move.h"
#include "move_generator/move_generation.h"

#include "zobrist.h"
#include "ttable.h"

#include <map>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct PerftResult {
    std::map<std::string, u64> detailed_data;
    std::map<std::string, u64> move_data;

    PerftResult();
    PerftResult(const json& detailed, const json& moves = nullptr);

    u64 get_detailed(const std::string& name) const;
    u64 get_move(const std::string& name) const;
    u64 getTotalNodes() const;

    void add_end_node();
    void add_checkmate();
    void add_moves(const std::string& algebraic_move, u64 node_count);
    void add_detailed(const Move& move);
};

PerftResult start_perft_test(const std::string& fen, unsigned depth);

template <Color color>
u64 perft_test(Board& board, PerftResult& results, unsigned depth /*, TranspositionTable& tt*/)
{
    if ( depth == 0 ) {
        return 1ULL;
    }

    // uint64_t hash = board.getZobristHash();

    u64 nodes = 0ULL;
    // if ( tt.lookup(hash, nodes) ) { return nodes; }

    MoveList move_list;
    generate_moves<color>(move_list, board);

    if ( move_list.size() == 0 ) {
        results.add_checkmate();
        return 0ULL;
    }

    for ( const auto& move : move_list ) {
        if ( depth == 1 ) {
            results.add_detailed(move);
        }

        board.move<color>(move);
        nodes += perft_test<utils::switchColor(color)>(board, results, depth - 1);
        board.undo<color>(move);
    }

    // tt.store(hash, nodes);
    return nodes;
}