#include "perft/perft.h"

PerftResult start_perft_test(const std::string& fen, unsigned depth)
{
    PerftResult results;

    if ( depth == 0 ) {
        results.add_end_node();
        return results;
    }

    Board board(fen);
    bool white_to_start = board.whiteTurn();

    MoveList move_list;
    if ( white_to_start ) {
        generate_moves<Color::white>(move_list, board);
    }
    else {
        generate_moves<Color::black>(move_list, board);
    }

    // TranspositionTable tt;

    if ( move_list.size() == 0 ) {
        results.add_checkmate();
        return results;
    }

    for ( const auto& move : move_list ) {
        if ( depth == 1 ) {
            results.add_detailed(move);
        }

        if ( white_to_start ) {
            board.move<Color::white>(move);
            results.add_moves(move.toLongAlgebraic(), perft_test<Color::black>(board, results, depth - 1));
            board.undo<Color::white>(move);
        }
        else {
            board.move<Color::black>(move);
            results.add_moves(move.toLongAlgebraic(), perft_test<Color::white>(board, results, depth - 1));
            board.undo<Color::black>(move);
        }
    }

    return results;
}

PerftResult::PerftResult()
{
    detailed_data = {
        {"total_nodes", 0}, {"captures", 0}, {"en_passants", 0},
        {"castles", 0}, {"promotions", 0}, {"checks", 0},
        {"discovery_checks", 0}, {"double_checks", 0}, {"checkmates", 0},
    };
}

PerftResult::PerftResult(const json& detailed, const json& moves)
{
    for ( const auto& item : detailed.items() ) {
        detailed_data[item.key()] = item.value();
    }

    for ( const auto& item : moves.items() ) {
        move_data[item.key()] = item.value();
    }
}

u64 PerftResult::get_detailed(const std::string& name) const
{
    if ( detailed_data.count(name) == 0 ) {
        std::cout << "detailed KEY NOT FOUND: " << name << '\n';
    }
    return detailed_data.at(name);
}

u64 PerftResult::get_move(const std::string& name) const
{
    if ( move_data.count(name) == 0 ) {
        std::cout << "moves KEY NOT FOUND: " << name << '\n';
    }
    return move_data.at(name);
}

void PerftResult::add_checkmate() { ++detailed_data["checkmates"]; }
void PerftResult::add_end_node() { ++detailed_data["total_nodes"]; }
void PerftResult::add_moves(const std::string& algebraic_move, u64 node_count) { move_data[algebraic_move] = node_count; }
u64 PerftResult::getTotalNodes() const { return detailed_data.at("total_nodes"); }

// TODO: add : "checks", "discovery_checks", "double_checks", "checkmates"
void PerftResult::add_detailed(const Move& move)
{
    ++detailed_data["total_nodes"];

    switch ( move.getFlag() ) {
        case Move::Flag::castle_k:
        case Move::Flag::castle_q: {
            ++detailed_data["castles"];
        } break;
        case Move::Flag::capture: {
            ++detailed_data["captures"];
        } break;
        case Move::Flag::ep: {
            ++detailed_data["captures"];
            ++detailed_data["en_passants"];
        } break;
        case Move::Flag::promo_x_n:
        case Move::Flag::promo_x_b:
        case Move::Flag::promo_x_r:
        case Move::Flag::promo_x_q: {
            ++detailed_data["captures"];
        } // fallthrough
        case Move::Flag::promo_n:
        case Move::Flag::promo_b:
        case Move::Flag::promo_r:
        case Move::Flag::promo_q: {
            ++detailed_data["promotions"];
        } break;
        default: break;
    }
}