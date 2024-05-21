#include "temp_cmd_manager.h"

template <Color color>
u64 perft_entry(Board& board, int depth);

template <Color color>
u64 perft(Board& board, int depth);

Move CommandManager::makeMoveFromString(const std::string& moveStr, const Board& board)
{
    if ( moveStr.length() < 4 ) {
        std::cerr << "Invalid move string: " << moveStr << std::endl;
        return Move();
    }

    std::string from_str = moveStr.substr(0, 2);
    std::string to_str = moveStr.substr(2, 2);
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
        if ( moveStr == "e1g1" || moveStr == "e8g8" ) {
            flag = Move::Flag::castle_k;
        }
        else if ( moveStr == "e1c1" || moveStr == "e8c8" ) {
            flag = Move::Flag::castle_q;
        }
    }

    if ( moveStr.length() > 4 ) {
        char promotionChar = moveStr[4];
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

void CommandManager::parseCommand()
{
    bool quit = false;
    std::string cmd;
    while ( !quit ) {
        if ( !std::getline(std::cin, cmd) ) {
            break;
        }
        std::istringstream ss(cmd);
        std::string token;
        ss >> token;
        if ( to_lower(token) == "quit" ) {
            quit = true;
        }
        else if ( token == "uci" ) {
            std::cout << "id name slou 1.1\n"
                << "id author amazzetta\n\n"
                << "uciok\n";
        }
        else if ( token == "stop" ) {
            quit = true;
        }
        else if ( token == "position" ) {
            ss >> token;
            if ( token == "startpos" ) {
                _fen = STARTPOS;
                board = Board(STARTPOS);
                ss >> token;
            }
            else if ( token == "fen" ) {
                std::string fen;
                while ( ss >> token && token != "moves" ) {
                    fen += token + " ";
                }

                board = Board(fen);
            }
            else {
                std::cout << "unknown command: " << token << '\n';
            }

            if ( token == "moves" ) {
                if ( board.whiteTurn() ) {
                    while ( ss >> token ) {
                        auto mv = makeMoveFromString(token, board);
                        std::cout << "move: " << token << '\n';
                        board.move<Color::white>(mv);
                    }
                }
                else {
                    while ( ss >> token ) {
                        auto mv = makeMoveFromString(token, board);
                        std::cout << "move: " << token << '\n';
                        board.move<Color::black>(mv);
                    }
                }
            }
        }
        else if ( token == "go" ) {
            ss >> token;
            if ( token == "perft" ) {
                ss >> token;
                int depth = std::stoi(token);
                if ( board.whiteTurn() ) {
                    perft_entry<Color::white>(board, depth);
                }
                else {
                    perft_entry<Color::white>(board, depth);
                }
            }
        }
        else if ( token == "isready" ) {
            std::cout << "readyok\n";
        }
        else if ( token == "print" || token == "d" ) {
            std::cout << board.toPrettyString(false, false) << '\n';
        }
        else {
            std::cout << "unknown command: " << token << '\n';
        }
    }
}