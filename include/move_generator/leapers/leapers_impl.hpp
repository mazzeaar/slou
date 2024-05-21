#include "leapers.h"

// ================================
// MOVE GENERATION FUNCTIONS
// ================================

template <Color color>
void leapers::pawn(MoveList& move_list, const Board& board)
{
    constexpr bool is_white = utils::isWhite(color);
    static constexpr int OFFSET_MOVE = (is_white) ? -8 : 8;
    static constexpr int OFFSET_PUSH = (is_white) ? -16 : 16;
    static constexpr int OFFSET_ATTACK_L = (is_white) ? -7 : 7;
    static constexpr int OFFSET_ATTACK_R = (is_white) ? -9 : 9;

    static constexpr uint64_t LEFT_FILE = (is_white) ? FILE_A : FILE_H;
    static constexpr uint64_t RIGHT_FILE = (is_white) ? FILE_H : FILE_A;
    static constexpr uint64_t PROMO_RANK = (is_white) ? RANK_7 : RANK_2;
    static constexpr uint64_t PUSH_RANK = (is_white) ? RANK_2 : RANK_7;

    const uint64_t occupancy = board.getOccupancy();
    const uint64_t enemy = board.getEnemy<color>();
    const uint64_t ep_field = board.getEpField();

    const uint64_t pawns = board.getPieces<PieceType::pawn, color>();

    const uint64_t move_pawns = pawns & ~PROMO_RANK;
    const uint64_t push_pawns = pawns & PUSH_RANK;

    // filter pawns that can not attack to l/r, this way we dont have to do bit 'teleportation' check
    const uint64_t attack_pawns_l = pawns & ~PROMO_RANK & ~LEFT_FILE;
    const uint64_t attack_pawns_r = pawns & ~PROMO_RANK & ~RIGHT_FILE;

    const uint64_t promotable_pawns = pawns & PROMO_RANK;
    // again, we filter pawns that can not attack to l/r
    const uint64_t promo_capture_l = promotable_pawns & ~LEFT_FILE;
    const uint64_t promo_capture_r = promotable_pawns & ~RIGHT_FILE;

    {
        u64 quiet = pawnMove<color>(move_pawns, occupancy);
        BIT_LOOP(quiet)
        {
            const uint64_t to = get_LSB(quiet);
            const uint64_t from = to + OFFSET_MOVE;
            move_list.add(Move::make<Move::Flag::quiet>(from, to));
        }
    }


    {
        u64 push = pawnPush<color>(push_pawns, occupancy);
        BIT_LOOP(push)
        {
            const uint64_t to = get_LSB(push);
            const uint64_t from = to + OFFSET_PUSH;
            move_list.add(Move::make<Move::Flag::pawn_push>(from, to));
        }
    }


    {
        uint64_t left_ep = pawnAttackLeft<color>(attack_pawns_l, ep_field);
        BIT_LOOP(left_ep)
        {
            const uint64_t to = get_LSB(left_ep);
            const uint64_t from = to + OFFSET_ATTACK_L;
            move_list.add(Move::make<Move::Flag::ep>(from, to));
        }

        uint64_t right_ep = pawnAttackRight<color>(attack_pawns_r, ep_field);
        BIT_LOOP(right_ep)
        {
            const uint64_t to = get_LSB(right_ep);
            const uint64_t from = to + OFFSET_ATTACK_R;
            move_list.add(Move::make<Move::Flag::ep>(from, to));
        }
    }


    {
        uint64_t left_attacks = pawnAttackLeft<color>(attack_pawns_l, enemy);
        BIT_LOOP(left_attacks)
        {
            const uint64_t to = get_LSB(left_attacks);
            const uint64_t from = to + OFFSET_ATTACK_L;
            move_list.add(Move::make<Move::Flag::capture>(from, to));
        }

        uint64_t right_attacks = pawnAttackRight<color>(attack_pawns_r, enemy);
        BIT_LOOP(right_attacks)
        {
            const uint64_t to = get_LSB(right_attacks);
            const uint64_t from = to + OFFSET_ATTACK_R;
            move_list.add(Move::make<Move::Flag::capture>(from, to));
        }
    }


    {
        uint64_t quiet_promo = pawnMove<color>(promotable_pawns, occupancy);
        BIT_LOOP(quiet_promo)
        {
            const uint64_t to = get_LSB(quiet_promo);
            const uint64_t from = to + OFFSET_MOVE;

            move_list.add(Move::make<Move::Flag::promo_n>(from, to));
            move_list.add(Move::make<Move::Flag::promo_b>(from, to));
            move_list.add(Move::make<Move::Flag::promo_r>(from, to));
            move_list.add(Move::make<Move::Flag::promo_q>(from, to));
        }

        uint64_t capture_left_promo = pawnAttackLeft<color>(promo_capture_l, enemy);
        BIT_LOOP(capture_left_promo)
        {
            const uint64_t to = get_LSB(capture_left_promo);
            const uint64_t from = to + OFFSET_ATTACK_L;

            move_list.add(Move::make<Move::Flag::promo_x_n>(from, to));
            move_list.add(Move::make<Move::Flag::promo_x_b>(from, to));
            move_list.add(Move::make<Move::Flag::promo_x_r>(from, to));
            move_list.add(Move::make<Move::Flag::promo_x_q>(from, to));
        }

        uint64_t capture_right_promo = pawnAttackRight<color>(promo_capture_r, enemy);
        BIT_LOOP(capture_right_promo)
        {
            const uint64_t to = get_LSB(capture_right_promo);
            const uint64_t from = to + OFFSET_ATTACK_R;

            move_list.add(Move::make<Move::Flag::promo_x_n>(from, to));
            move_list.add(Move::make<Move::Flag::promo_x_b>(from, to));
            move_list.add(Move::make<Move::Flag::promo_x_r>(from, to));
            move_list.add(Move::make<Move::Flag::promo_x_q>(from, to));
        }
    }
}

template <Color color>
void leapers::knight(MoveList& move_list, const Board& board)
{
    const uint64_t occupancy = board.getOccupancy();
    const uint64_t enemy = board.getEnemy<color>();

    uint64_t knights = board.getPieces<PieceType::knight, color>();
    BIT_LOOP(knights)
    {
        const uint64_t from = get_LSB(knights);

        uint64_t move_targets = knight_attacks[from] & ~occupancy;
        BIT_LOOP(move_targets)
        {
            const uint64_t to = get_LSB(move_targets);
            move_list.add(Move::make<Move::Flag::quiet>(from, to));
        }

        uint64_t attack_targets = knight_attacks[from] & enemy;
        BIT_LOOP(attack_targets)
        {
            const uint64_t to = get_LSB(attack_targets);
            move_list.add(Move::make<Move::Flag::capture>(from, to));
        }
    }
}

template <Color color>
void leapers::king(MoveList& move_list, const Board& board, u64 enemy_attacks)
{
    const u64 occupancy = board.getOccupancy();
    const u64 enemy = board.getEnemy<color>();

    u64 king = board.getPieces<PieceType::king, color>();
    const uint64_t from = get_LSB(king);

    u64 moves = king_attacks[from] & ~occupancy & ~enemy_attacks;
    BIT_LOOP(moves)
    {
        const uint64_t to = get_LSB(moves);
        move_list.add(Move::make<Move::Flag::quiet>(from, to));
    }

    u64 attacks = king_attacks[from] & enemy;
    BIT_LOOP(attacks)
    {
        const uint64_t to = get_LSB(attacks);
        move_list.add(Move::make<Move::Flag::capture>(from, to));
    }

    // TODO: this can be removed through state template
    if ( board.canCastle<color>() ) {
        if ( board.canCastleKs<color>(enemy_attacks) ) {
            move_list.add(Move::make<Move::Flag::castle_k>(from, from + 2));
        }

        if ( board.canCastleQs<color>(enemy_attacks) ) {
            move_list.add(Move::make<Move::Flag::castle_q>(from, from - 2));
        }
    }
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

template <Color color>
inline u64 leapers::generatePawnMask(u64 pawns)
{
    if constexpr ( utils::isWhite(color) ) {
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

template <Color color>
inline u64 leapers::pawnMove(u64 pawns, u64 occupancy)
{
    if constexpr ( utils::isWhite(color) ) {
        return north(pawns) & ~occupancy;
    }
    else {
        return south(pawns) & ~occupancy;
    }
}

template <Color color>
inline u64 leapers::pawnPush(u64 pawns, u64 occupancy)
{
    if constexpr ( utils::isWhite(color) ) {
        return north(north(pawns & RANK_2) & ~occupancy) & ~occupancy;
    }
    else {
        return south(south(pawns & RANK_7) & ~occupancy) & ~occupancy;
    }
}

template <Color color>
inline u64 leapers::pawnAttackLeft(u64 pawns, u64 enemy)
{
    if constexpr ( utils::isWhite(color) ) {
        return unsafe_north_west(pawns) & enemy;
    }
    else {
        return unsafe_south_east(pawns) & enemy;
    }
}

template <Color color>
inline u64 leapers::pawnAttackRight(u64 pawns, u64 enemy)
{
    if constexpr ( utils::isWhite(color) ) {
        return unsafe_north_east(pawns) & enemy;
    }
    else {
        return unsafe_south_west(pawns) & enemy;
    }
}