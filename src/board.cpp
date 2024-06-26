#include "board/board.h"
#include "board/board.hpp"
#include <sstream>

Board::Board(const std::string& fen)
{
    state = new State();

    state->ep_field = 0ULL;

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
                placePiece<Color::white>(piece, square);
            }
            else {
                placePiece<Color::black>(piece, square);
            }

            state->mailbox[square] = piece;

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
                    state->cur_color = Color::white;
                }
                else if ( token == "b" ) {
                    state->cur_color = Color::black;
                }
                else {
                    assert(false && "COLOR NOT ALLOWED");
                }
            } break;
            case 1: { // castling rights
                if ( token == "-" ) {
                    state->castling_rights.raw = 0x00;
                    break;
                }

                if ( token.find('K') == std::string::npos )
                    state->castling_rights.white_ks = 0;
                if ( token.find('Q') == std::string::npos )
                    state->castling_rights.white_qs = 0;
                if ( token.find('k') == std::string::npos )
                    state->castling_rights.black_ks = 0;
                if ( token.find('q') == std::string::npos )
                    state->castling_rights.black_qs = 0;
            } break;
            case 2: { // ep target
                if ( token != "-" ) {
                    int square = utils::coordinateToIndex(token);
                    state->ep_field = single_bit_u64(square);
                }
                else {
                    state->ep_field = 0ULL;
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

    state->zobrist_hash = Zobrist::computeHash(*this);
}

std::string Board::getFen() const
{
    std::string res = "";

    for ( int j = 7; j >= 0; --j ) {
        int counter = 0;
        for ( unsigned i = 0; i < 8; ) {
            while ( state->mailbox[(j * 8) + i] == Piece::none && i < 8 ) {
                ++counter;
                ++i;
            }

            if ( counter != 0 ) {
                res += std::to_string(counter);
                counter = 0;
            }
            else {
                res += utils::PieceToChar(state->mailbox[(j * 8) + i]);
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

    if ( state->ep_field == 0ULL ) {
        res += "- ";
    }
    else {
        res += utils::square_to_coordinates[(get_LSB(state->ep_field))];
        res += " ";
    }

    res += std::to_string(state->half_move_clock) + " ";
    res += std::to_string(state->full_move_clock);

    return res;
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
            str += utils::PieceToChar(state->mailbox[square]);;
            str += " " + VERTICAL_BORDER;
        }

        str += " " + std::to_string(rank);
    }

    str += "\n" + BORDER + BOTTOM_ROW;

    return str;
}
