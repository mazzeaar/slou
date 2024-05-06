#include "board.h"
#include <sstream>

Board::Board(const std::string& fen)
{
    updateEpField(0ULL);
    for ( int i = 0; i < 12; ++i ) {
        pieces[i] = 0ULL;
    }

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
            // Move to the next rank
            rank--;
            file = 0;
            index++;
        }
        else if ( isdigit(c) ) {
            // Skip over empty squares
            file += c - '0';
            index++;
        }
        else {
            // Place a piece on the board
            const int square = (rank * 8) + file;
            const type::Piece piece = type::getPiece(c);
            setPiece(piece, square);
            file++;
            index++;
        }
    }

    // Remaining parts of FEN: active color, castling rights, en passant, halfmove, fullmove
    std::string rest_fen = fen.substr(fen.find_first_of(' ') + 1);
    std::istringstream iss(rest_fen);
    std::string token;
    int part = 0;
    while ( iss >> token ) {
        switch ( part ) {
            case 0: { // active color
                if ( token == "w" ) {
                    cur_color = type::Color::white;
                }
                else if ( token == "b" ) {
                    cur_color = type::Color::black;
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
                    removeCastleKs(type::Color::white);
                if ( token.find('Q') == std::string::npos )
                    removeCastleQs(type::Color::white);
                if ( token.find('k') == std::string::npos )
                    removeCastleKs(type::Color::black);
                if ( token.find('q') == std::string::npos )
                    removeCastleQs(type::Color::black);
            } break;
            case 2: { // ep target
                if ( token != "-" ) {
                    int file = token[0] - 'a';
                    int rank = token[1] - '0';

                    updateEpField(single_bit_u64((8 * rank) + file));
                }
                else {
                    updateEpField(0ULL);
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
    if ( canCastleKs(type::Color::white) ) castling += "K";
    if ( canCastleQs(type::Color::white) ) castling += "Q";
    if ( canCastleKs(type::Color::black) ) castling += "k";
    if ( canCastleQs(type::Color::black) ) castling += "q";
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
        res += type::square_to_coordinates[(get_LSB(ep_field))];
        res += " ";
    }

    res += std::to_string(half_move_clock) + " ";
    res += std::to_string(full_move_clock);

    return res;
}

// hacky way to disable castling rights;
void Board::tryToRemoveCastlingRights(const Move& move)
{
    if ( type::getPieceType(move_history.top().moving_piece) == type::PieceType::rook ) {
        if ( move.getFrom() == 0 ) {
            removeCastleQs(type::Color::white);
        }
        else if ( move.getFrom() == 7 ) {
            removeCastleKs(type::Color::white);
        }
        else if ( move.getFrom() == 56 ) {
            removeCastleQs(type::Color::black);
        }
        else if ( move.getFrom() == 63 ) {
            removeCastleKs(type::Color::black);
        }
    }

    if ( type::getPieceType(move_history.top().moving_piece) == type::PieceType::king ) {
        if ( move.getFrom() == 4 ) {
            removeCastleKs(type::Color::white);
            removeCastleQs(type::Color::white);
        }
        else if ( move.getFrom() == 60 ) {
            removeCastleKs(type::Color::black);
            removeCastleQs(type::Color::black);
        }
    }

    if ( type::getPieceType(move_history.top().captured_piece) == type::PieceType::rook ) {
        if ( move.getTo() == 0 ) {
            removeCastleQs(type::Color::white);
        }
        else if ( move.getTo() == 7 ) {
            removeCastleKs(type::Color::white);
        }
        else if ( move.getTo() == 56 ) {
            removeCastleQs(type::Color::black);
        }
        else if ( move.getTo() == 63 ) {
            removeCastleKs(type::Color::black);
        }
    }
}

void Board::storeState(const Move& move)
{
    MoveState state;

    state.moving_piece = getPieceFromSquare(move.getFrom());

    if ( move.isCapture() ) {
        state.captured_piece = getPieceFromSquare(move.getTo());
    }

    state.ep_field_before = getEpField();
    state.castling_rights = castling_rights.raw;

    move_history.push(state);
    /*
    FullState state;

    state.moving_piece = getPieceFromSquare(move.getFrom());

    if ( move.isCapture() ) {
        state.captured_piece = getPieceFromSquare(move.getTo());
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

            const type::Piece piece = getPieceFromSquare(square);
            str += type::PieceToUnicode(piece);
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
    for ( unsigned i = 0; i < mailbox.size(); ++i ) {
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
            case 0: {
                str += "Position: ";
            } break;
            case 1: {
                str += "Turn: ";
                str += (type::isWhite(cur_color)) ? "white" : "black";
            } break;
            case 2: {

            } break;
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
            const type::Piece piece = getPieceFromSquare(square);
            mailbox[rank][file] = type::PieceToChar(piece);
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

            const type::Piece piece = getPieceFromSquare(square);

            if ( emoji ) {
                mailbox[rank][file] = type::PieceToUnicode(piece);
            }
            else {
                mailbox[rank][file] = type::PieceToChar(piece);
            }
        }
    }

    return mailbox;
}

void Board::move(const Move& move)
{
    DEBUG_START;

    storeState(move);

    auto state = move_history.top();

    switch ( move.getFlag() ) {
        case MoveFlag::QUIET_MOVE:
        case MoveFlag::DOUBLE_PAWN_PUSH:
        case MoveFlag::KING_CASTLE:
        case MoveFlag::QUEEN_CASTLE:
        case MoveFlag::CAPTURE:
        case MoveFlag::EN_PASSANT: {
            move_piece(state.moving_piece, move.getFrom(), move.getTo());
        } break;
        default: break;
    }

    switch ( move.getFlag() ) {
        case MoveFlag::QUIET_MOVE: {
            tryToRemoveCastlingRights(move);
        } break;
        case MoveFlag::DOUBLE_PAWN_PUSH: {
            if ( type::isWhite(cur_color) ) {
                updateEpField(north(single_bit_u64(move.getFrom())));
            }
            else {
                updateEpField(south(single_bit_u64(move.getFrom())));
            }

            cur_color = type::switchColor(cur_color);
            DEBUG_END;
            return; // early exit because we set the ep field
        } break;
        case MoveFlag::KING_CASTLE: {
            if ( type::isWhite(cur_color) ) {
                move_piece(type::Piece::R, 7, 5);
                removeCastle(type::Color::white);
            }
            else {
                move_piece(type::Piece::r, 63, 61);
                removeCastle(type::Color::black);
            }
        } break;
        case MoveFlag::QUEEN_CASTLE: {
            if ( type::isWhite(cur_color) ) {
                move_piece(type::Piece::R, 0, 3);
                removeCastle(type::Color::white);
            }
            else {
                move_piece(type::Piece::r, 56, 59);
                removeCastle(type::Color::black);
            }
        } break;
        case MoveFlag::CAPTURE: {
            remove_piece(state.captured_piece, move.getTo());
            tryToRemoveCastlingRights(move);
        } break;
        case MoveFlag::EN_PASSANT: {
            if ( type::isWhite(cur_color) ) {
                remove_piece(type::Piece::p, move.getTo() - 8);
            }
            else {
                remove_piece(type::Piece::P, move.getTo() + 8);
            }
        } break;
        case MoveFlag::KNIGHT_PROMOTION:
        case MoveFlag::BISHOP_PROMOTION:
        case MoveFlag::ROOK_PROMOTION:
        case MoveFlag::QUEEN_PROMOTION:
        case MoveFlag::KNIGHT_PROMO_CAPTURE:
        case MoveFlag::BISHOP_PROMO_CAPTURE:
        case MoveFlag::ROOK_PROMO_CAPTURE:
        case MoveFlag::QUEEN_PROMO_CAPTURE: {
            state.promotion_piece = type::getPiece(move.getPromotionPieceType(), cur_color);
            move_history.top().promotion_piece = state.promotion_piece;

            if ( move.isCapture() ) {
                remove_piece(state.captured_piece, move.getTo());
                tryToRemoveCastlingRights(move);
            }

            remove_piece(state.moving_piece, move.getFrom());
            place_piece(state.promotion_piece, move.getTo());
        } break;
        default: break;
    }

    updateEpField(0ULL);
    cur_color = type::switchColor(cur_color);

    DEBUG_END;
}

void Board::undo(const Move& move)
{
    DEBUG_START;

    if ( move_history.empty() ) {
        LOG_ERROR << "MOVE HISTORY IS EMPTY!\n";
        throw std::runtime_error("mh is empty\n");
    }

    // restoreState();
    // DEBUG_END;
    // return;

    MoveState last_state = move_history.top();
    move_history.pop();

    cur_color = type::switchColor(cur_color);
    updateEpField(last_state.ep_field_before);
    castling_rights.raw = last_state.castling_rights;

    // undo normal moves
    switch ( move.getFlag() ) {
        case MoveFlag::QUIET_MOVE:
        case MoveFlag::DOUBLE_PAWN_PUSH:
        case MoveFlag::KING_CASTLE:
        case MoveFlag::QUEEN_CASTLE:
        case MoveFlag::CAPTURE:
        case MoveFlag::EN_PASSANT: {
            move_piece(last_state.moving_piece, move.getTo(), move.getFrom());
        } break;
        default: break;
    }

    if ( move.isCastle() ) {
        if ( move.getFlag() == MoveFlag::KING_CASTLE ) {
            if ( type::isWhite(cur_color) ) {
                move_piece(type::Piece::R, 5, 7);
            }
            else {
                move_piece(type::Piece::r, 61, 63);
            }
        }
        else if ( move.getFlag() == MoveFlag::QUEEN_CASTLE ) {
            if ( type::isWhite(cur_color) ) {
                move_piece(type::Piece::R, 3, 0);
            }
            else {
                move_piece(type::Piece::r, 59, 56);
            }
        }

        DEBUG_END;
        return;
    }

    if ( move.isEnpassant() ) {
        if ( type::isWhite(cur_color) ) {
            place_piece(type::Piece::p, move.getTo() - 8);
        }
        else {
            place_piece(type::Piece::P, move.getTo() + 8);
        }

        DEBUG_END;
        return;
    }

    if ( move.isCapture() ) {
        place_piece(last_state.captured_piece, move.getTo());
    }

    if ( move.isPromotion() ) {
        remove_piece(last_state.promotion_piece, move.getTo());
        place_piece(last_state.moving_piece, move.getFrom());
    }

    DEBUG_END;
}