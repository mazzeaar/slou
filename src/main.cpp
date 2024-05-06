#include <iostream>
#include <chrono>
#include <iomanip>
#include <string>

#include "move_generator/move_generation.h"
#include "perft/perft_testing.h"

DEBUG_INIT;
static const std::string engine_prefix = "> slou: ";

template <type::Color color>
u64 debug_perft(Board& board, int depth)
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

    std::cout << "\nNodes searched: " << nodes << '\n';
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

inline void speedtest(const std::string& fen, int depth, const u64 expected)
{
    Board board(fen);
    u64 nodes = 0ULL;

    auto begin = std::chrono::high_resolution_clock::now();
    if ( board.whiteTurn() ) {
        nodes = perft<type::Color::white>(board, depth);
    }
    else {
        nodes = perft<type::Color::black>(board, depth);
    }
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    std::cout << nodes << " nodes in " << duration << "ms\n"
        << "NPS = " << (nodes * 1000) / (duration) << '\n'
        << "expected: " << expected << '\n'
        << "diff: " << (expected > nodes ? expected - nodes : nodes - expected) << '\n'<<'\n';
}

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

// Helper to print a single row
template<typename T>
void printRow(const T& value)
{
    std::cout << std::right << std::setw(20) << value;
}

template<typename T, typename... Args>
void printRow(const T& first, Args... args)
{
    std::cout << std::right << std::setw(20) << first;
    printRow(args...);  // Recursive call
}

// Function to print the header or any row with variable number of columns
template<typename... Args>
void printTableLine(Args... args)
{
    printRow(args...);
    std::cout << std::endl;
}

// Separator lines for clarity in table
void printSeparator(int count)
{
    for ( int i = 0; i < count; i++ ) {
        std::cout << "--------------------";
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[])
{
    if ( argc > 1 ) {
        if ( strcmp(argv[1], "--perft") == 0 ) {
            PerftTestSuite tests;
            tests.runTests();
        }

        if ( strcmp(argv[1], "--position") == 0 ) {
            std::string fen = argv[2];
            fen = fen.substr(1, fen.size() - 2);
            std::string n = argv[3];
            speedtest(fen, std::stoi(n), std::stoi(argv[4]));
        }

        if ( strcmp(argv[1], "--debug") == 0 ) {
            std::string fen = argv[2];
            fen = fen.substr(1, fen.size() - 2);
            int depth = std::stoi(argv[3]);

            Board board(fen);

            if ( board.whiteTurn() ) {
                debug_perft<type::Color::white>(board, depth);
            }
            else {
                debug_perft<type::Color::black>(board, depth);
            }
        }
    }

    magic::initMagics();

    // printBenchmarkInfo();
    return 0;
}