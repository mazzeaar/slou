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
}

std::string Board::getFen() const
{
    std::string res = "";
    auto mailbox = toCharMailbox();

    for ( int j = 7; j >= 0; --j ) {
        int counter = 0;
        for ( unsigned i = 0; i < mailbox[j].size(); ) {
            while ( mailbox[j][i] == ' ' ) {
                ++counter;
                ++i;
            }

            if ( counter != 0 ) {
                res += std::to_string(counter);
                counter = 0;
            }
            else {
                res += mailbox[j][i];
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

    state.moving_piece = getPiece(move.getFrom());

    if ( move.isCapture() ) {
        state.captured_piece = getPiece(move.getTo());
    }

    state.ep_field_before = getEpField();
    state.castling_rights = castling_rights.raw;

    move_history.push(state);
    /*
    FullState state;

    state.moving_piece = getPiece(move.getFrom());

    if ( move.isCapture() ) {
        state.captured_piece = getPiece(move.getTo());
    }

    state.s_color = cur_color;
    for ( int i = 0; i < 12; ++i ) {
        state.s_pieces[i] = pieces[i];
    }
    state.s_ep_field = ep_field;
    state.s_castling_rights = castling_rights.raw;
    move_history.push(state);
    */
}

void Board::restoreState()
{
    /*
    FullState state = move_history.top();
    move_history.pop();

    cur_color = state.s_color;
    for ( int i = 0; i < 12; ++i ) {
        pieces[i] = state.s_pieces[i];
    }

    ep_field = state.s_ep_field;
    castling_rights.raw = state.s_castling_rights;
    */
}

std::string Board::toString() const
{
    std::string str = "---------------\n";

    for ( int rank = 7; rank >= 0; --rank ) {
        for ( int file = 0; file < 8; ++file ) {
            const int square = (rank * 8) + file;

            const Piece piece = getPiece(square);
            str += utils::PieceToUnicode(piece);
            str += " ";
        }

        str += "\n";
    }

    return str + "---------------\n";
}

std::string Board::toPrettyString(bool colored, bool emoji) const
{
    const std::string BORDER = "  +---+---+---+---+---+---+---+---+\n";
    const char VERTICAL_BORDER = '|';


    auto mailbox = toStringMailbox(emoji);
    std::string str = "\n    a   b   c   d   e   f   g   h\n"; // Column labels

    bool white_cell = true;
    int rank = 8;
    for ( int i = mailbox.size() - 1; i >= 0; --i ) {
        const auto& row = mailbox[i];
        str += BORDER;
        str += std::to_string(rank) + " " + VERTICAL_BORDER;

        for ( const auto& s : row ) {
            if ( colored && white_cell ) {
                str += WHITE_BG;
            }
            white_cell = !white_cell;

            str += " " + s + " " + RESET + VERTICAL_BORDER;

        }

        str += " " + std::to_string(rank--);

        // swap white bg
        white_cell = !white_cell;

        str += "    ";


        switch ( i ) {
            case 7: {
                str += "Position: " + getFen();
            } break;
            case 6: {
                str += "Turn: ";
                str += (utils::isWhite(cur_color)) ? "white" : "black";
            } break;
            case 5: {
                str += "Eval: TODO";
            } break;
            case 4: {
                str += "HMC/FMC: TODO";
            }
        }

        str += "\n";
    }

    str += BORDER; // Add the final horizontal border
    str += "    a   b   c   d   e   f   g   h\n"; // Column labels

    return str;
}

std::vector<std::vector<char>> Board::toCharMailbox() const
{
    std::vector<std::vector<char>> mailbox = std::vector<std::vector<char>>(8, std::vector<char>(8, ' '));

    for ( int rank = 7; rank >= 0; --rank ) {
        for ( int file = 0; file < 8; ++file ) {
            const int square = (rank * 8) + file;
            const Piece piece = getPiece(square);
            mailbox[rank][file] = utils::PieceToChar(piece);
        }
    }

    return mailbox;
}

std::vector<std::vector<std::string>> Board::toStringMailbox(bool emoji) const
{
    std::vector<std::vector<std::string>> mailbox = std::vector<std::vector<std::string>>(8, std::vector<std::string>(8, " "));

    for ( int rank = 7; rank >= 0; --rank ) {
        for ( int file = 0; file < 8; ++file ) {
            const int square = (rank * 8) + file;

            const Piece piece = getPiece(square);

            if ( emoji ) {
                mailbox[rank][file] = utils::PieceToUnicode(piece);
            }
            else {
                mailbox[rank][file] = utils::PieceToChar(piece);
            }
        }
    }

    return mailbox;
}
