#include "game.h"

Game::Game(const std::string& fen)
{
    if ( fen == "startpos" ) {
        board = Board();
    }
    else {
        try {
            board = Board(fen);
        }
        catch ( std::exception& e ) {
            throw std::string("Failed to parse the fen!");
        }
    }
}

void Game::make_move(const std::string& algebraic_move)
{
    const Move move = moveFromSring(algebraic_move);

    if ( board.whiteTurn() ) {
        board.move<Color::white>(move);
    }
    else {
        board.move<Color::black>(move);
    }
}

void Game::unmake_move(const std::string& algebraic_move)
{
    const Move move = moveFromSring(algebraic_move);

    if ( board.whiteTurn() ) {
        board.undo<Color::white>(move);
    }
    else {
        board.undo<Color::black>(move);
    }
}

uint64_t Game::perftSimpleEntry(int depth)
{
    constexpr bool print_moves = false;
    if ( board.whiteTurn() ) {
        return perft<Color::white, print_moves>(board, depth);
    }
    else {
        return perft<Color::black, print_moves>(board, depth);
    }
}

uint64_t Game::perftDetailEntry(int depth)
{
    constexpr bool print_moves = true;
    if ( board.whiteTurn() ) {
        return debug_perft<Color::white, print_moves>(board, depth);
    }
    else {
        return debug_perft<Color::black, print_moves>(board, depth);
    }
}

Move Game::moveFromSring(const std::string& algebraic_move)
{
    if ( algebraic_move.length() < 4 ) {
        std::cerr << "Invalid move string: " << algebraic_move << std::endl;
        return Move();
    }

    std::string from_str = algebraic_move.substr(0, 2);
    std::string to_str = algebraic_move.substr(2, 2);
    uint8_t from = utils::coordinateToIndex(from_str);
    uint8_t to = utils::coordinateToIndex(to_str);

    // default flag
    Move::Flag flag = Move::Flag::quiet;
    const PieceType from_piece = board.getPieceType(from);
    const PieceType to_piece = board.getPieceType(to);

    if ( to_piece != PieceType::none ) {
        if ( get_LSB(board.getEpField()) == to ) {
            flag = Move::Flag::ep;
        }
        else {
            flag = Move::Flag::capture;
        }
    }
    else if ( from_piece == PieceType::pawn ) {
        if ( 8 <= from && from < 16 && (to - from == 16) ) {
            flag = Move::Flag::pawn_push;
        }
        else if ( 48 <= from && from < 56 && (from - to == 16) ) {
            flag = Move::Flag::pawn_push;
        }
    }
    else if ( from_piece == PieceType::king ) {
        if ( algebraic_move == "e1g1" || algebraic_move == "e8g8" ) {
            flag = Move::Flag::castle_k;
        }
        else if ( algebraic_move == "e1c1" || algebraic_move == "e8c8" ) {
            flag = Move::Flag::castle_q;
        }
    }

    if ( algebraic_move.length() > 4 ) {
        char promotionChar = algebraic_move[4];
        switch ( tolower(promotionChar) ) {
            case 'q': {
                if ( flag == Move::Flag::capture ) {
                    flag = Move::Flag::promo_x_q;
                }
                else {
                    flag = Move::Flag::promo_q;
                }
            } break;
            case 'r': {
                if ( flag == Move::Flag::capture ) {
                    flag = Move::Flag::promo_x_r;
                }
                else {
                    flag = Move::Flag::promo_r;
                }
            } break;
            case 'b': {
                if ( flag == Move::Flag::capture ) {
                    flag = Move::Flag::promo_x_b;
                }
                else {
                    flag = Move::Flag::promo_b;
                }
            } break;
            case 'n': {
                if ( flag == Move::Flag::capture ) {
                    flag = Move::Flag::promo_x_n;
                }
                else {
                    flag = Move::Flag::promo_n;
                }
            } break;
            default: std::cerr << "Invalid promotion piece: " << promotionChar << std::endl;
        }
    }

    return Move(from, to, flag);
}