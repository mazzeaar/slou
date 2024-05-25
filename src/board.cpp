#include "board/board.h"
#include "board/board.hpp"
#include <sstream>

Board::Board(const std::string& fen)
{
    ep_field = 0ULL;

    std::fill(mailbox.begin(), mailbox.end(), Piece::none);

    std::string board_fen = fen.substr(0, fen.find_first_of(' '));
    unsigned index = 0;
    int rank = 7;
    int file = 0;

    while ( rank >= 0 ) {
        if ( index >= board_fen.size() ) {
            break;
        }

        char c = board_fen[index];

        if ( c == '/' ) {
            // next rank
            rank--;
            file = 0;
            index++;
        }
        else if ( isdigit(c) ) {
            // skip empty squares
            file += c - '0';
            index++;
        }
        else {
            // place a piece
            const int square = (rank * 8) + file;
            const Piece piece = utils::getPiece(c);
            if ( utils::isWhite(piece) ) {
                place_piece<Color::white>(piece, square);
            }
            else {
                place_piece<Color::black>(piece, square);
            }

            mailbox[square] = piece;

            file++;
            index++;
        }
    }

    std::string rest_fen = fen.substr(fen.find_first_of(' ') + 1);
    std::istringstream iss(rest_fen);
    std::string token;
    int part = 0;
    while ( iss >> token ) {
        switch ( part ) {
            case 0: { // active color
                if ( token == "w" ) {
                    cur_color = Color::white;
                }
                else if ( token == "b" ) {
                    cur_color = Color::black;
                }
                else {
                    assert(false && "COLOR NOT ALLOWED");
                }
            } break;
            case 1: { // castling rights
                if ( token == "-" ) {
                    castling_rights.raw = 0x00;
                    break;
                }

                if ( token.find('K') == std::string::npos )
                    castling_rights.white_ks = 0;
                if ( token.find('Q') == std::string::npos )
                    castling_rights.white_qs = 0;
                if ( token.find('k') == std::string::npos )
                    castling_rights.black_ks = 0;
                if ( token.find('q') == std::string::npos )
                    castling_rights.black_qs = 0;
            } break;
            case 2: { // ep target
                if ( token != "-" ) {
                    int square = utils::coordinateToIndex(token);
                    ep_field = single_bit_u64(square);
                }
                else {
                    ep_field = 0ULL;
                }

            } break;
            case 3: { // Halfmove clock
                // Handle halfmove clock
            } break;
            case 4: { // Fullmove number
                // Handle fullmove number
            } break;
        }

        ++part;
    }

    zobrist_hash = Zobrist::computeHash(*this);
}

std::string Board::getFen() const
{
    std::string res = "";

    for ( int j = 7; j >= 0; --j ) {
        int counter = 0;
        for ( unsigned i = 0; i < 8; ) {
            while ( mailbox[(j * 8) + i] == Piece::none && i < 8 ) {
                ++counter;
                ++i;
            }

            if ( counter != 0 ) {
                res += std::to_string(counter);
                counter = 0;
            }
            else {
                res += utils::PieceToChar(mailbox[(j * 8) + i]);
                ++i;
            }
        }

        if ( j != 0 ) {
            res += "/";
        }
    }

    res += " ";

    if ( whiteTurn() ) {
        res += "w ";
    }
    else {
        res += "b ";
    }

    std::string castling = "";
    if ( canCastleKs<Color::white>() ) castling += "K";
    if ( canCastleQs<Color::white>() ) castling += "Q";
    if ( canCastleKs<Color::black>() ) castling += "k";
    if ( canCastleQs<Color::black>() ) castling += "q";
    if ( castling == "" ) {
        res += "- ";
    }
    else {
        res += castling + " ";
    }

    if ( ep_field == 0ULL ) {
        res += "- ";
    }
    else {
        res += utils::square_to_coordinates[(get_LSB(ep_field))];
        res += " ";
    }

    res += std::to_string(half_move_clock) + " ";
    res += std::to_string(full_move_clock);

    return res;
}

void Board::storeState(const Move& move)
{
    MoveState state;

    const uint64_t from = move.getFrom();
    const uint64_t to = move.getTo();
    const bool is_capture = move.isCapture();

    state.moving_piece = getPiece(from);

    if ( is_capture ) {
        state.captured_piece = getPiece(to);
    }

    state.ep_field_before = getEpField();
    state.castling_rights = castling_rights.raw;
    state.hash = zobrist_hash;

    move_history.push(state);
}

std::string Board::toString() const
{
    const std::string BORDER = "+---+---+---+---+---+---+---+---+\n";
    const std::string VERTICAL_BORDER = "|";
    const std::string BOTTOM_ROW = "  a   b   c   d   e   f   g   h\n";
    std::string str = getFen() + "\n";

    for ( unsigned rank = 8; rank > 0; --rank ) {
        str += "\n" + BORDER + VERTICAL_BORDER;
        const unsigned row_begin = (rank - 1) * 8;
        for ( unsigned square = row_begin; square < row_begin + 8; ++square ) {
            str += ' ';
            str += utils::PieceToChar(mailbox[square]);;
            str += " " + VERTICAL_BORDER;
        }

        str += " " + std::to_string(rank);
    }

    str += "\n" + BORDER + BOTTOM_ROW;

    return str;
}
