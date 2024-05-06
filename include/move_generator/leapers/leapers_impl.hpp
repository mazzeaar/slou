#include "leapers.h"
#include "debug.h"

inline void leapers::addMoves(MoveList& move_list, int source_square, u64 moves, MoveFlag flag)
{
    while ( moves ) {
        const int target_square = pop_LSB(moves);
        move_list.add(Move(source_square, target_square, flag));
    }
}

// ================================
// MOVE GENERATION FUNCTIONS
// ================================

template <type::Color color>
void leapers::pawn(MoveList& move_list, const Board& board)
{
    DEBUG_START;

    const u64 occupancy = board.getOccupancy();
    const u64 enemy = board.getEnemy(color);
    const u64 ep_field = board.getEpField();

    if ( get_bit_count(ep_field) > 1 ) {
        LOG_ERROR << "can not have more than 1 piece on the ep_field!\n";
        throw std::runtime_error("can not have more than 1 piece on the ep_field!\n");
    }

    u64 pawns = board.getPawns(color);

    // ignore promo square
    if constexpr ( type::isWhite(color) ) pawns &= ~RANK_7;
    else pawns &= ~RANK_2;

    u64 quiet = pawnMove<color>(pawns, occupancy);
    for ( int target; quiet; quiet &= quiet - 1 ) {
        target = get_LSB(quiet);
        if constexpr ( type::isWhite(color) ) {
            move_list.add(Move::make<MoveFlag::QUIET_MOVE>(target - 8, target));
        }
        else {
            move_list.add(Move::make<MoveFlag::QUIET_MOVE>(target + 8, target));
        }
    }

    u64 push = pawnPush<color>(pawns, occupancy);
    for ( int target; push; push &= push - 1 ) {
        target = get_LSB(push);
        if constexpr ( type::isWhite(color) ) {
            move_list.add(Move::make<MoveFlag::DOUBLE_PAWN_PUSH>(target - 16, target));
        }
        else {
            move_list.add(Move::make<MoveFlag::DOUBLE_PAWN_PUSH>(target + 16, target));
        }
    }

    while ( pawns ) {
        u64 source = pop_lsb_to_u64(pawns);
        int source_square = get_LSB(source);

        u64 attack_table;
        if constexpr ( type::isWhite(color) ) attack_table = white_pawn_attacks[source_square];
        else attack_table = black_pawn_attacks[source_square];

        u64 attacks = attack_table & enemy;
        leapers::addMoves(move_list, source_square, attacks, MoveFlag::CAPTURE);

        u64 ep = attack_table & ep_field;
        leapers::addMoves(move_list, source_square, ep, MoveFlag::EN_PASSANT);

        // u64 quiet = pawnMove<color>(source, occupancy);
        // leapers::add_moves(move_list, source_square, quiet, MoveFlag::QUIET_MOVE);

        // u64 push = pawnPush<color>(source, occupancy);
        // leapers::add_moves(move_list, source_square, push, MoveFlag::DOUBLE_PAWN_PUSH);
    }

    u64 promo_pawns = board.getPawns(color);
    if constexpr ( type::isWhite(color) ) promo_pawns &= RANK_7;
    else promo_pawns &= RANK_2;

    while ( promo_pawns ) {
        u64 source = pop_lsb_to_u64(promo_pawns);
        int source_square = get_LSB(source);

        u64 attack_table;
        if ( type::isWhite(color) ) attack_table = white_pawn_attacks[source_square];
        else attack_table = black_pawn_attacks[source_square];

        u64 promo_captures = attack_table & enemy;
        while ( promo_captures ) {
            const int target_square = pop_LSB(promo_captures);
            move_list.add(Move(source_square, target_square, MoveFlag::KNIGHT_PROMO_CAPTURE));
            move_list.add(Move(source_square, target_square, MoveFlag::BISHOP_PROMO_CAPTURE));
            move_list.add(Move(source_square, target_square, MoveFlag::ROOK_PROMO_CAPTURE));
            move_list.add(Move(source_square, target_square, MoveFlag::QUEEN_PROMO_CAPTURE));
        }

        u64 promotions = pawnMove<color>(source, occupancy);
        while ( promotions ) {
            const int target_square = pop_LSB(promotions);
            move_list.add(Move(source_square, target_square, MoveFlag::KNIGHT_PROMOTION));
            move_list.add(Move(source_square, target_square, MoveFlag::BISHOP_PROMOTION));
            move_list.add(Move(source_square, target_square, MoveFlag::ROOK_PROMOTION));
            move_list.add(Move(source_square, target_square, MoveFlag::QUEEN_PROMOTION));
        }
    }

    DEBUG_END;
}

template <type::Color color>
void leapers::knight(MoveList& move_list, const Board& board)
{
    DEBUG_START;
    const u64 occupancy = board.getOccupancy();
    const u64 enemy = board.getEnemy(color);

    u64 knights = board.getKnights(color);
    while ( knights ) {
        u64 origin_bit = pop_lsb_to_u64(knights);
        const int from = get_LSB(origin_bit);

        u64 move_targets = knight_attacks[from] & ~occupancy;
        leapers::addMoves(move_list, from, move_targets, MoveFlag::QUIET_MOVE);

        u64 attack_targets = knight_attacks[from] & enemy;
        leapers::addMoves(move_list, from, attack_targets, MoveFlag::CAPTURE);
    }
    DEBUG_END;
}

template <type::Color color>
void leapers::king(MoveList& move_list, const Board& board, u64 enemy_attacks)
{
    DEBUG_START;
    const u64 occupancy = board.getOccupancy();
    const u64 enemy = board.getEnemy(color);

    u64 king = board.getKing(color);
    const int source_square = get_LSB(king);

    u64 moves = king_attacks[source_square] & ~occupancy;
    leapers::addMoves(move_list, source_square, moves, MoveFlag::QUIET_MOVE);

    u64 attacks = king_attacks[source_square] & enemy;
    leapers::addMoves(move_list, source_square, attacks, MoveFlag::CAPTURE);

    if ( board.canCastleKs(color, enemy_attacks) ) {
        move_list.add(Move(source_square, source_square + 2, MoveFlag::KING_CASTLE));
    }

    if ( board.canCastleQs(color, enemy_attacks) ) {
        move_list.add(Move(source_square, source_square - 2, MoveFlag::QUEEN_CASTLE));
    }
    DEBUG_END;
}

// ================================
// MASK GENERATORS
// ================================

inline u64 leapers::generateKingMask(u64 king)
{
    const u64 up = north_west(king) | north(king) | north_east(king);
    const u64 down = south_west(king) | south(king) | south_east(king);
    const u64 left = west(king);
    const u64 right = east(king);

    return up | down | left | right;
}

inline u64 leapers::generateKnightMask(u64 knights)
{
    const u64 up_left = ((knights & ~(RANK_78 | FILE_A)) << 15);
    const u64 up_right = ((knights & ~(RANK_78 | FILE_H)) << 17);
    const u64 up = up_left | up_right;

    const u64 down_left = ((knights & ~(RANK_12 | FILE_A)) >> 17);
    const u64 down_right = ((knights & ~(RANK_12 | FILE_H)) >> 15);
    const u64 down = down_left | down_right;

    const u64 right_up = ((knights & ~(FILE_GH | RANK_8)) << 10);
    const u64 right_down = ((knights & ~(FILE_GH | RANK_1)) >> 6);
    const u64 right = right_up | right_down;

    const u64 left_up = ((knights & ~(FILE_AB | RANK_8)) << 6);
    const u64 left_down = ((knights & ~(FILE_AB | RANK_1)) >> 10);
    const u64 left = left_up | left_down;

    return up | down | left | right;
}

template <type::Color color>
inline u64 leapers::generatePawnMask(u64 pawns)
{
    if constexpr ( type::isWhite(color) ) {
        const u64 left = north_west(pawns);
        const u64 right = north_east(pawns);
        return (left | right);
    }
    else {
        const u64 left = south_west(pawns);
        const u64 right = south_east(pawns);
        return (left | right);
    }
}

template <type::Color color>
inline u64 leapers::pawnMove(u64 pawns, u64 occupancy)
{
    if constexpr ( type::isWhite(color) ) {
        return north(pawns) & ~occupancy;
    }
    else {
        return south(pawns) & ~occupancy;
    }
}

template <type::Color color>
inline u64 leapers::pawnPush(u64 pawns, u64 occupancy)
{
    if constexpr ( type::isWhite(color) ) {
        return north(north(pawns & RANK_2) & ~occupancy) & ~occupancy;
    }
    else {
        return south(south(pawns & RANK_7) & ~occupancy) & ~occupancy;
    }
}

