#pragma once

#include <array>

#include "definitions.h"
#include "board/board.h"
#include "move_generator/move_generation.h"

static constexpr double INFTY = std::numeric_limits<double>::infinity();

constexpr std::array<int, 64> flipTable(const std::array<int, 64>& table)
{
    std::array<int, 64> flipped {};
    for ( int i = 0; i < 64; ++i ) {
        flipped[i] = table[63 - i];
    }

    return flipped;
}

static constexpr std::array<int, 64> pawn_position_score = {
    0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5,  5, 10, 25, 25, 10,  5,  5,
    0, 0,  0, 20, 20,  0,  0,  0,
    5, -5,-10,  0,  0,-10, -5,  5,
    5, 10, 10,-20,-20, 10, 10,  5,
    0, 0, 0, 0, 0, 0, 0, 0
};

static constexpr std::array<int, 64> knight_position_score = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50,
};

static constexpr std::array<int, 64> bishop_position_score = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20,
};

static constexpr std::array<int, 64> rook_position_score = {
    0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    0,  0,  0,  5,  5,  0,  0,  0
};

static constexpr std::array<int, 64> queen_position_score = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
    -5,  0,  5,  5,  5,  5,  0, -5,
    0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

// early game
static constexpr std::array<int, 64> king_position_score = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
    20, 20,  0,  0,  0,  0, 20, 20,
    20, 30, 10,  0,  0, 10, 30, 20
};

template <PieceType type, Color color>
const std::array<int, 64> getPositionBoard()
{
    if constexpr ( utils::isWhite(color) ) {
        switch ( type ) {
            case PieceType::pawn: return pawn_position_score;
            case PieceType::knight: return knight_position_score;
            case PieceType::bishop: return bishop_position_score;
            case PieceType::rook: return rook_position_score;
            case PieceType::queen: return queen_position_score;
            case PieceType::king: return king_position_score;
            default: return {};
        }
    }
    else {
        switch ( type ) {
            case PieceType::pawn: return flipTable(pawn_position_score);
            case PieceType::knight: return flipTable(knight_position_score);
            case PieceType::bishop: return flipTable(bishop_position_score);
            case PieceType::rook: return flipTable(rook_position_score);
            case PieceType::queen: return flipTable(queen_position_score);
            case PieceType::king: return flipTable(king_position_score);
            default: return {};
        }
    }
}

template <Color color>
constexpr int getMaterialScore(const Board& board)
{
    return (get_bit_count((board.getPieces<PieceType::pawn, color>())) * 100)
        + (get_bit_count((board.getPieces<PieceType::knight, color>())) * 320)
        + (get_bit_count((board.getPieces<PieceType::bishop, color>())) * 320)
        + (get_bit_count((board.getPieces<PieceType::rook, color>())) * 500)
        + (get_bit_count((board.getPieces<PieceType::queen, color>())) * 900)
        + (get_bit_count((board.getPieces<PieceType::king, color>())) * 10000);
}

template <PieceType type, Color color>
constexpr int getPiecePositionScore(const Board& board)
{
    auto score_table = getPositionBoard<type, color>();

    int result = 0;
    uint64_t pieces = board.getPieces<type, color>();
    BIT_LOOP(pieces)
    {
        const int square = get_LSB(pieces);
        result += score_table[square];
    }

    if constexpr ( utils::isWhite(color) ) {
        return result;
    }
    else {
        return -result;
    }
}

template <Color color>
constexpr int getPositionalScore(const Board& board)
{
    int result = 0;
    result += getPiecePositionScore<PieceType::pawn, color>(board);
    result += getPiecePositionScore<PieceType::knight, color>(board);
    result += getPiecePositionScore<PieceType::bishop, color>(board);
    result += getPiecePositionScore<PieceType::rook, color>(board);
    result += getPiecePositionScore<PieceType::queen, color>(board);
    result += getPiecePositionScore<PieceType::king, color>(board);

    return result;
}

template <Color color>
constexpr double evalPosition(Board& board)
{
    const int material_score = getMaterialScore<color>(board);
    const int position_score = getPositionalScore<color>(board);
    const double score = material_score + position_score;
    if constexpr ( utils::isWhite(color) ) {
        return score;
    }
    else {
        return -score;
    }
}

template <Color color>
double minimax(Board& board, int depth, double alpha, double beta)
{
    if ( depth == 0 ) {
        return evalPosition<color>(board);
    }

    MoveList move_list;
    generate_moves<color>(move_list, board);
    const uint64_t enemy_attacks = generate_attacks<color>(board);

    if ( move_list.size() == 0 ) {
        if ( board.isCheck<color>(enemy_attacks) ) {
            return (color == Color::white) ? -INFTY : INFTY;
        }
        else {
            return 0;
        }
    }

    double best_score = (color == Color::white) ? -INFTY : INFTY;
    for ( const auto& move : move_list ) {
        board.move<color>(move);
        double score = minimax<utils::switchColor(color)>(board, depth - 1, alpha, beta);
        board.undo<color>(move);

        if constexpr ( color == Color::white ) {
            best_score = std::max(best_score, score);
            alpha = std::max(alpha, score);
        }
        else {
            best_score = std::min(best_score, score);
            beta = std::min(beta, score);
        }

        if ( beta <= alpha ) {
            break;
        }
    }

    return best_score;
}

template <Color color>
Move getBestMove(Board& board, int depth = 5)
{
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

    return best_move;
}
