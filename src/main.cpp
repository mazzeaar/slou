#include <iostream>
#include <chrono>
#include <iomanip>
#include <string>
#include <sstream>
#include <cctype>

#include "move_generator/move_generation.h"
#include "perft/perft_testing.h"

DEBUG_INIT;

template <Color color>
u64 perft(Board& board, int depth)
{
    MoveList move_list;
    u64 nodes = generate_moves<color>(move_list, board);

    if ( depth == 1 ) {
        return nodes;
    }

    nodes = 0ULL;
    for ( const auto& move : move_list ) {
        board.move<color>(move);
        nodes += perft<utils::switchColor(color)>(board, depth - 1);
        board.undo<color>(move);
    }

    return nodes;
}

template <Color color>
u64 perft_entry(Board& board, int depth)
{
    MoveList move_list;
    u64 nodes = generate_moves<color>(move_list, board);

    if ( depth == 1 ) {
        return nodes;
    }

    nodes = 0ULL;
    for ( const auto& move : move_list ) {
        board.move<color>(move);
        u64 move_nodes = perft<utils::switchColor(color)>(board, depth - 1);
        board.undo<color>(move);

        std::cout << move.toLongAlgebraic() << ' ' << move_nodes << std::endl;
        nodes += move_nodes;
    }

    std::cout << '\n' << nodes << '\n';
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
        uint8_t from = utils::coordinateToIndex(from_str);
        uint8_t to = utils::coordinateToIndex(to_str);

        // default flag
        Move::Flag flag = Move::Flag::quiet;
        const PieceType from_piece = board.getPieceTypeFromSquare(from);
        const PieceType to_piece = board.getPieceTypeFromSquare(to);

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
};

Move makeMoveFromString(const std::string& moveStr, const Board& board)
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
    const PieceType from_piece = board.getPieceTypeFromSquare(from);
    const PieceType to_piece = board.getPieceTypeFromSquare(to);

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

/*
 * Producer Consumer Pattern for UCI.
 *
 * Producer has stop, consumers can read. consumer stops as soon as possible.
 *
 */
int main(int argc, char** argv)
{
    Zobrist::initialize();

    if ( argc > 1 ) {
        if ( strcmp(argv[1], "-v") == 0 ) {
            int depth = std::stoi(argv[2]);
            std::string fen = argv[3];
            fen = fen.substr(1, fen.size() - 1);
            u64 expected = std::stoull(argv[4]);
            Board board(fen);
            u64 nodes = 0ULL;
            //std::cout << "here depth=" << depth << ", fen=" << fen << ", exp=" << expected << '\n';
            if ( board.whiteTurn() ) {
                nodes = perft<Color::white>(board, depth);
            }
            else {
                nodes = perft<Color::black>(board, depth);
            }

            if ( nodes != expected ) {
                std::cout << RED << "WRONG: " << RESET << fen << ", got=" << nodes << ", expected=" << expected << '\n';
            }
            else {
                std::cout << GREEN << "Passed: " << RESET << fen << ", got=" << nodes << ", expected=" << expected << '\n';
            }
        }
        else if ( strcmp(argv[1], "--perft") == 0 ) {
            PerftTestSuite tests;
            tests.runTests();
        }
        else {
            Board board;

            std::string fen;
            int depth = std::stoi(argv[1]);
            int i = 2;
            for ( ; i < argc; ++i ) {
                if ( strcmp(argv[i], "moves") == 0 ) {
                    break;
                }

                fen += argv[i];
                fen += " ";
            }

            board = Board(fen);
            if ( strcmp(argv[i], "moves") == 0 ) {
                for ( i += 1; i < argc; ++i ) {
                    Move mv = makeMoveFromString(argv[i], board);
                    if ( board.whiteTurn() ) {
                        board.move<Color::white>(mv);
                    }
                    else {
                        board.move<Color::black>(mv);
                    }
                }
            }

            u64 nodes = 0ULL;
            if ( board.whiteTurn() ) {
                nodes = perft_entry<Color::white>(board, depth);
            }
            else {
                nodes = perft_entry<Color::black>(board, depth);
            }

            std::cout << std::endl;
            std::cout << nodes << std::endl;
        }
    }
    else {
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
    }

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
                        perft_entry<Color::white>(board, depth);
                    }
                    else {
                        perft_entry<Color::black>(board, depth);
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