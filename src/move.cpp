#include "move.h"

// https://www.chessprogramming.org/Algebraic_Chess_Notation#Long_Algebraic_Notation_.28LAN.29
std::string Move::toLongAlgebraic() const
{
    //if ( flag == MoveFlag::KING_CASTLE ) return "O-O";
    //if ( flag == MoveFlag::QUEEN_CASTLE ) return "O-O-O";

    std::string moveStr = idxToNotation(getFrom());
    moveStr += isCapture() ? "" : "";
    moveStr += idxToNotation(getTo());

    if ( getFlag() == MoveFlag::QUEEN_PROMOTION || getFlag() == MoveFlag::QUEEN_PROMO_CAPTURE ) {
        moveStr += "q";
    }
    else if ( getFlag() == MoveFlag::ROOK_PROMOTION || getFlag() == MoveFlag::ROOK_PROMO_CAPTURE ) {
        moveStr += "r";
    }
    else if ( getFlag() == MoveFlag::BISHOP_PROMOTION || getFlag() == MoveFlag::BISHOP_PROMO_CAPTURE ) {
        moveStr += "b";
    }
    else if ( getFlag() == MoveFlag::KNIGHT_PROMOTION || getFlag() == MoveFlag::KNIGHT_PROMO_CAPTURE ) {
        moveStr += "n";
    }

    return moveStr;
}

std::string Move::toString() const
{
    std::ostringstream stream;

    stream << "Move from " << idxToNotation(getFrom())
        << " to " << idxToNotation(getTo());

    if ( isCapture() ) {
        stream << " with capture";
    }

    if ( isPromotion() ) {
        stream << "with promotion to ";
        switch ( getFlag() ) {
            case MoveFlag::QUEEN_PROMOTION: {
                stream << "Queen";
            } break;
            case MoveFlag::ROOK_PROMOTION: {
                stream << "Rook";
            } break;
            case MoveFlag::BISHOP_PROMOTION: {
                stream << "Bishop";
            } break;
            case MoveFlag::KNIGHT_PROMOTION: {
                stream << "Knight";
            } break;
            default: {
                std::cerr << "...interesting, we found a wrong flag in Move::toString()!\n";
            }
        }
    }

    if ( isEnpassant() ) {
        stream << " as en passant";
    }

    if ( isCastle() ) {
        if ( getFlag() == MoveFlag::KING_CASTLE ) stream << " as king-side castle";
        if ( getFlag() == MoveFlag::QUEEN_CASTLE ) stream << " as queen-side castle";
    }

    return stream.str();
}