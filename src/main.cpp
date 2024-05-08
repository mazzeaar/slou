#include <iostream>
#include <chrono>
#include <iomanip>
#include <string>
#include <sstream>
#include <cctype>

#include "move_generator/move_generation.h"
#include "perft/perft_testing.h"

DEBUG_INIT;

template <type::Color color>
u64 perft_entry(Board& board, int depth)
{
    MoveList move_list;
    u64 nodes = generate_moves<color>(move_list, board);

    if ( depth == 1 ) {
        return nodes;
    }

    nodes = 0ULL;
    for ( const auto& move : move_list ) {
        board.move(move);
        u64 move_nodes = perft<type::switchColor(color)>(board, depth - 1);
        board.undo(move);

        std::cout << move.toLongAlgebraic() << ": " << move_nodes << '\n';
        nodes += move_nodes;
    }

    std::cout << "\nNodes searched: " << nodes << "\n\n";
    return nodes;
}

template <type::Color color>
u64 perft(Board& board, int depth)
{
    MoveList move_list;
    u64 nodes = generate_moves<color>(move_list, board);

    if ( depth == 1 ) {
        return nodes;
    }

    nodes = 0ULL;
    for ( const auto& move : move_list ) {
        board.move(move);
        nodes += perft<type::switchColor(color)>(board, depth - 1);
        board.undo(move);
    }

    return nodes;
}
class CommandManager {
private:
    std::string _fen = STARTPOS;
    Board board = Board(_fen);
    std::string& to_lower(std::string& s) { for ( char c : s ) { c = std::tolower(c); } return s; }

    Move makeMoveFromString(const std::string& moveStr, const Board& board)
    {
        if ( moveStr.length() < 4 ) {
            std::cerr << "Invalid move string: " << moveStr << std::endl;
            return Move();
        }

        std::string from_str = moveStr.substr(0, 2);
        std::string to_str = moveStr.substr(2, 2);
        uint8_t from = type::coordinateToIndex(from_str);
        uint8_t to = type::coordinateToIndex(to_str);

        // default flag
        MoveFlag flag = MoveFlag::QUIET_MOVE;
        const type::PieceType from_piece = board.getPieceTypeFromSquare(from);
        const type::PieceType to_piece = board.getPieceTypeFromSquare(to);

        if ( to_piece != type::PieceType::none ) {
            if ( get_LSB(board.getEpField()) == to ) {
                flag = MoveFlag::EN_PASSANT;
            }
            else {
                flag = MoveFlag::CAPTURE;
            }
        }
        else if ( from_piece == type::PieceType::pawn ) {
            if ( 8 <= from && from < 16 && (to - from == 16) ) {
                flag = MoveFlag::DOUBLE_PAWN_PUSH;
            }
            else if ( 48 <= from && from < 56 && (from - to == 16) ) {
                flag = MoveFlag::DOUBLE_PAWN_PUSH;
            }
        }
        else if ( from_piece == type::PieceType::king ) {
            if ( moveStr == "e1g1" || moveStr == "e8g8" ) {
                flag = MoveFlag::KING_CASTLE;
            }
            else if ( moveStr == "e1c1" || moveStr == "e8c8" ) {
                flag = MoveFlag::QUEEN_CASTLE;
            }
        }

        if ( moveStr.length() > 4 ) {
            char promotionChar = moveStr[4];
            switch ( tolower(promotionChar) ) {
                case 'q': {
                    if ( flag == MoveFlag::CAPTURE ) {
                        flag = MoveFlag::QUEEN_PROMO_CAPTURE;
                    }
                    else {
                        flag = MoveFlag::QUEEN_PROMOTION;
                    }
                } break;
                case 'r': {
                    if ( flag == MoveFlag::CAPTURE ) {
                        flag = MoveFlag::ROOK_PROMO_CAPTURE;
                    }
                    else {
                        flag = MoveFlag::ROOK_PROMOTION;
                    }
                } break;
                case 'b': {
                    if ( flag == MoveFlag::CAPTURE ) {
                        flag = MoveFlag::BISHOP_PROMO_CAPTURE;
                    }
                    else {
                        flag = MoveFlag::BISHOP_PROMOTION;
                    }
                } break;
                case 'n': {
                    if ( flag == MoveFlag::CAPTURE ) {
                        flag = MoveFlag::KNIGHT_PROMO_CAPTURE;
                    }
                    else {
                        flag = MoveFlag::KNIGHT_PROMOTION;
                    }
                } break;
                default: std::cerr << "Invalid promotion piece: " << promotionChar << std::endl;
            }
        }

        return Move(from, to, flag);
    }

public:
    CommandManager() = default;

    void parseCommand()
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
                    while ( ss >> token ) {
                        auto mv = makeMoveFromString(token, board);
                        board.move(mv);
                    }
                }
            }
            else if ( token == "go" ) {
                ss >> token;
                if ( token == "perft" ) {
                    ss >> token;
                    int depth = std::stoi(token);
                    if ( board.whiteTurn() ) {
                        perft_entry<type::Color::white>(board, depth);
                    }
                    else {
                        perft_entry<type::Color::white>(board, depth);
                    }
                }
            }
            else if ( token == "isready" ) {
                std::cout << "readyok\n";
            }
            else if ( token == "print" || token == "d" ) {
                std::cout << board.toPrettyString() << '\n';
            }
            else {
                std::cout << "unknown command: " << token << '\n';
            }
        }
    }
};
/*
 * Producer Consumer Pattern for UCI.
 *
 * Producer has stop, consumers can read. consumer stops as soon as possible.
 *
 */
int main()
{
    std::cout << '\n'
        << "  ********   **           *******     **     **\n"
        << " **//////   /**          **/////**   /**    /**\n"
        << "/**         /**         **     //**  /**    /**\n"
        << "/*********  /**        /**      /**  /**    /**\n"
        << "////////**  /**        /**      /**  /**    /**\n"
        << "       /**  /**        //**     **   /**    /**\n"
        << " ********   /********   //*******    //*******\n"
        << "////////    ////////     ///////      ///////\n\n"
        << "A chess egine by ama - 2024\n\n"
        << "try 'help' if you are lost <3\n\n";

    CommandManager cmd_manager;

    cmd_manager.parseCommand();

    return 0;
}

/*
Board board;
    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::string line;
    while ( std::getline(std::cin, line) ) {
        std::stringstream ss;
        ss << line;

        std::string token;
        while ( ss >> token ) {
            if ( token == "position" ) {
                ss >> token;
                if ( token == "startpos" ) {
                    continue;
                }
                else if ( token == "fen" ) {
                    fen = "";
                    while ( ss >> token ) {
                        if ( token == "moves" ) {
                            LOG_ERROR << "not implemented yet :)\n";
                            break;
                        }

                        fen += token + " ";
                    }

                    board = Board(fen);
                }
                else {
                    std::cout << "usage: " << "position [fen <fenstring> | startpos ]  moves <move1> .... <movei>\n";
                }
            }
            else if ( token == "print" || token == "d" ) {
                std::cout << board.toPrettyString() << '\n';
            }
            else if ( token == "go" ) {
                ss >> token;
                if ( token == "perft" ) {
                    ss >> token;
                    int depth;
                    try {
                        depth = std::stoi(token);
                    }
                    catch ( std::invalid_argument& e ) {
                        std::cout << "usage: " << "go perft <x>\n";
                        break;
                    }

                    if ( board.whiteTurn() ) {
                        perft_entry<type::Color::white>(board, depth);
                    }
                    else {
                        perft_entry<type::Color::black>(board, depth);
                    }
                }
            }
            else if ( token == "help" ) {
                std::cout << "- position [fen <fenstring> | startpos ]  moves <move1> .... <movei>\n"
                    << "- go perft <x>\n"
                    << "- print | d\n";
            }
            else {
                std::cout << "try: help\n";
            }
        }
    }
*/