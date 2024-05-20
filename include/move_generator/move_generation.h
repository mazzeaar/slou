/**
 * @file move_generation.h
 * @author Aaron Mazzetta (amazzetta@ethz.ch)
 * @brief   the move generator works with three core functions:
 * We first generate all pseudolegal_moves, then filter them by using generate_moves and GenerateEnemyAttacks.
 *
 * @version 0.1
 * @date 2024-04-21
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <vector>
#include <iostream>

#include "definitions.h"

#include "leapers/leapers.h"
#include "sliders/sliders.h"
#include "board/board.h"
#include "move.h"

#include "zobrist.h"

static bool initialized_stuff = false;

inline void initializePrecomputedStuff()
{
    magic::initMagics();
    initLeapers();
    Zobrist::initialize();
}

/**
 * @brief               This function generates all (pseudo-) possible moves for this position,
 *                      even illegal ones. We will filter the list later.
 *
 * @tparam color        Player for whom we are generating moves
 * @param move_list     A container that can store our generated moves
 * @param board         The current board representation
 */
template <Color color>
inline u64 pseudolegal_moves(MoveList& move_list, const Board& board)
{
    DEBUG_START;

    const u64 enemy_attacks = generate_attacks<utils::switchColor(color)>(board);

    leapers::pawn<color>(move_list, board);
    leapers::knight<color>(move_list, board);
    leapers::king<color>(move_list, board, enemy_attacks);

    sliders::generateMoves<PieceType::bishop, color>(move_list, board);
    sliders::generateMoves<PieceType::rook, color>(move_list, board);
    sliders::generateMoves<PieceType::queen, color>(move_list, board);

    DEBUG_END;
    return move_list.size();
}

template <Color color>
inline u64 generate_moves(MoveList& move_list, Board& board)
{
    DEBUG_START;

    pseudolegal_moves<color>(move_list, board);

    for ( size_t i = 0; i < move_list.size(); ) {
        board.move<color>(move_list[i]);

        const u64 enemy_attacks = generate_attacks<utils::switchColor(color)>(board);

        if ( board.isCheck<color>(enemy_attacks) ) {
            board.undo<color>(move_list[i]);
            move_list.remove(i);
        }
        else {
            board.undo<color>(move_list[i]);
            ++i;
        }
    }

    DEBUG_END;
    return move_list.size();
}

/**
 * @brief   Generates a bitboard containing all fields that enemies can attack
 *
 * @tparam enemyColor   color of the enemy
 * @param board         a board
 * @return u64          the ORed enemy attacks
 */
template <Color color>
inline u64 generate_attacks(const Board& board)
{
    DEBUG_START;
    u64 attacks = 0ULL;
    const u64 occupancy = board.getOccupancy();

    const u64 pawns = board.getBoard<PieceType::pawn, color>();
    const u64 knights = board.getBoard<PieceType::knight, color>();
    const u64 king = board.getBoard<PieceType::king, color>();

    const u64 bishops = board.getBoard<PieceType::bishop, color>();
    const u64 rooks = board.getBoard<PieceType::rook, color>();
    const u64 queens = board.getBoard<PieceType::queen, color>();

    attacks |= leapers::getPawnAttackMask<color>(pawns);
    attacks |= leapers::getKnightAttackMask(knights);
    attacks |= leapers::getKingAttackMask(king);

    attacks |= sliders::getBitboard<PieceType::bishop>(bishops, occupancy);
    attacks |= sliders::getBitboard<PieceType::rook>(rooks, occupancy);
    attacks |= sliders::getBitboard<PieceType::queen>(queens, occupancy);

    DEBUG_END;
    return attacks;
}
