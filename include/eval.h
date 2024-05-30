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

template <PieceType type>
inline int getPieceScore(const Board& board, int value)
{
    const uint64_t white_pieces = board.getPieces<type, Color::white>();
    const uint64_t black_pieces = board.getPieces<type, Color::black>();
    return (get_bit_count(white_pieces) - get_bit_count(black_pieces)) * value;
}

inline int getMaterialScore(const Board& board)
{
    const int pawn_score = getPieceScore<PieceType::pawn>(board, 100);
    const int knight_score = getPieceScore<PieceType::knight>(board, 320);
    const int bishop_score = getPieceScore<PieceType::bishop>(board, 320);
    const int rook_score = getPieceScore<PieceType::rook>(board, 500);
    const int queen_score = getPieceScore<PieceType::queen>(board, 900);
    const int king_score = getPieceScore<PieceType::king>(board, 10000);

    return pawn_score + knight_score + bishop_score + rook_score + queen_score + king_score;
}

inline int getPawnScore(const Board& board)
{
    const uint64_t white_pawns = board.getPieces<PieceType::pawn, Color::white>();
    const uint64_t black_pawns = board.getPieces<PieceType::pawn, Color::black>();

    int double_pawns = 0;
    for ( uint64_t i = 0; i < 8; ++i ) {
        const uint64_t file_mask = (FILE_A << i);

        int w_pawns_in_file = get_bit_count(white_pawns & file_mask);
        int b_pawns_in_file = get_bit_count(black_pawns & file_mask);

        if ( w_pawns_in_file > 1 ) {
            double_pawns += w_pawns_in_file;
        }

        if ( b_pawns_in_file > 1 ) {
            double_pawns -= b_pawns_in_file;
        }
    }

    return -double_pawns;
}

template <PieceType type, Color color>
inline int getPiecePositionScore(const Board& board)
{
    const auto score_table = getPositionBoard<type, color>();

    int result = 0;
    uint64_t pieces = board.getPieces<type, color>();
    BIT_LOOP(pieces)
    {
        const int square = get_LSB(pieces);
        result += score_table[square];
    }

    return utils::isWhite(color) ? result : -result;
}

template <Color color>
inline int getPositionalScore(const Board& board)
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
inline double evalPosition(Board& board)
{
    const int material_score = getMaterialScore(board);
    const int position_score = getPositionalScore<color>(board);
    const int pawn_scores = getPawnScore(board);

    const double score = material_score + position_score + pawn_scores;

    if constexpr ( utils::isWhite(color) ) {
        return score;
    }
    else {
        return -score;
    }
}
