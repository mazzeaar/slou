#pragma once

#include "definitions.h"
#include "bitboard.h"

#include "move.h"
#include "board/board.h"
#include <array>

inline bool initialized_leapers;
inline std::array<u64, 64> white_pawn_attacks;
inline std::array<u64, 64> black_pawn_attacks;
inline std::array<u64, 64> knight_attacks;
inline std::array<u64, 64> king_attacks;

class leapers {
public:
    template <Color color>
    static inline void knight(MoveList& move_list, const Board& board);

    template <Color color>
    static inline void pawn(MoveList& move_list, const Board& board);

    template <Color color>
    static inline void king(MoveList& move_list, const Board& board, u64 enemy_attacks);

    template <Color color>
    static inline u64 generatePawnMask(u64 pawns);
    static inline u64 generateKnightMask(u64 knights);
    static inline u64 generateKingMask(u64 king);

    template <Color color>
    static inline u64 getPawnAttackMask(u64 pawns) { return generatePawnMask<color>(pawns); }

    static inline u64 getKnightAttackMask(u64 knights)
    {
        uint64_t result = 0ULL;
        BIT_LOOP(knights)
        {
            const int from = get_LSB(knights);
            result |= knight_attacks[from];
        }
        return result;
    }

    static inline u64 getKingAttackMask(u64 king)
    {
        const int from = get_LSB(king);
        const uint64_t result = king_attacks[from];
        return result;
    }


private:
    template <Color color>
    static inline u64 pawnMove(u64 pawns, u64 occupancy);

    template <Color color>
    static inline u64 pawnPush(u64 pawns, u64 occupancy);

    template <Color color>
    static inline u64 pawnAttackLeft(u64 pawns, u64 occupancy);

    template <Color color>
    static inline u64 pawnAttackRight(u64 pawns, u64 occupancy);
};

inline void initLeapers()
{
    if ( initialized_leapers ) {
        return;
    }

    for ( int i = 0; i < 64; ++i ) {
        white_pawn_attacks[i] = leapers::generatePawnMask<Color::white>(single_bit_u64(i));;
        black_pawn_attacks[i] = leapers::generatePawnMask<Color::black>(single_bit_u64(i));
        knight_attacks[i] = leapers::generateKnightMask(single_bit_u64(i));
        king_attacks[i] = leapers::generateKingMask(single_bit_u64(i));
    }

    initialized_leapers = true;
}

#include "leapers_impl.hpp"