#pragma once

#include "bitboard.h"
#include "board.h"
#include "move.h"
#include "move_generator/move_generation.h"

#include "zobrist.h"

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

template <type::Color color>
u64 perft_test(Board& board, PerftResult& results, unsigned depth)
{
    if ( depth == 0 ) {
        return 1ULL;
    }

    // u64 hash = Zobrist::computeHash(board);
    // u64 nodesCount = 0ULL;
    // if ( Zobrist::has(hash, nodesCount) && depth != 1 ) {
    //     return nodesCount;
    // }

    u64 nodes = 0ULL;
    MoveList move_list;
    generate_moves<color>(move_list, board);
    //pseudolegal_moves<color>(move_list, board);
    if ( move_list.size() == 0 ) {
        results.add_checkmate();
        return 0ULL;
    }

    for ( const auto& move : move_list ) {
        if ( depth == 1 ) {
            results.add_detailed(move);
        }

        board.move(move);
        nodes += perft_test<type::switchColor(color)>(board, results, depth - 1);
        board.undo(move);
    }

    // Zobrist::insert(hash, nodes);
    return nodes;
}