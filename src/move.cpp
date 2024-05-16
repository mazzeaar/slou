#include "move.h"

// https://www.chessprogramming.org/Algebraic_Chess_Notation#Long_Algebraic_Notation_.28LAN.29
std::string Move::toLongAlgebraic() const
{
    std::string moveStr = idxToNotation(getFrom());
    moveStr += isCapture() ? "" : "";
    moveStr += idxToNotation(getTo());

    if ( getFlag() == Move::Flag::promo_q || getFlag() == Move::Flag::promo_x_q ) {
        moveStr += "q";
    }
    else if ( getFlag() == Move::Flag::promo_r || getFlag() == Move::Flag::promo_x_r ) {
        moveStr += "r";
    }
    else if ( getFlag() == Move::Flag::promo_b || getFlag() == Move::Flag::promo_x_b ) {
        moveStr += "b";
    }
    else if ( getFlag() == Move::Flag::promo_n || getFlag() == Move::Flag::promo_x_n ) {
        moveStr += "n";
    }

    return moveStr;
}