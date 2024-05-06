#include "perft/perft_testing.h"

void print_table_header()
{
    std::cout
        << std::left << std::setw(COL_SPACING) << "NAME"
        << std::left << std::setw(COL_SPACING) << "GOT"
        << std::left << std::setw(COL_SPACING) << "EXPECTED"
        << std::left << std::setw(COL_SPACING) << "DIFFERENCE"
        << '\n';
}

void print_failed_row(const std::string& name, u64 got, u64 expected, int difference)
{
    std::cout << RED
        << std::left << std::setw(COL_SPACING) << name
        << std::left << std::setw(COL_SPACING) << got
        << std::left << std::setw(COL_SPACING) << expected
        << RESET
        << std::left << std::setw(COL_SPACING) << difference
        << '\n';
}

void print_passed_row(const std::string& name, u64 got)
{
    std::cout
        << std::left << std::setw(COL_SPACING) << name
        << std::left << std::setw(COL_SPACING) << got
        << '\n';
}

// ==========================================
//  DEPTHTEST
// ==========================================
DepthTest::DepthTest(const std::string& fen, const std::string& depth, json data)
    : fen(fen), depth(std::stoi(depth)), detailed(data["detailed"])
{
    if ( data.contains("moves") ) {
        has_move_data = true;
        moves = data["moves"];
    }

    expected = PerftResult(detailed, moves);
}

void DepthTest::run_test()
{
    result = start_perft_test(fen, depth);
    eval();
}

void DepthTest::eval()
{
    for ( const auto& [key, value] : expected.detailed_data ) {
        if ( skip_key(key) ) continue;

        int tmp;
        if ( expected.get_detailed(key) > result.get_detailed(key) ) {
            tmp = -1 * static_cast<int>(expected.get_detailed(key) - result.get_detailed(key));
        }
        else {
            tmp = static_cast<int>(result.get_detailed(key) - expected.get_detailed(key));
        }
        evaluated_detailed[key] = tmp;

        if ( tmp ) {
            passed_detailed = false;
            passed_all = false;
        }
    }

    if ( has_move_data ) {
        for ( const auto& [key, value] : expected.move_data ) {

            if ( result.move_data.count(key) == 0 ) {
                missing_moves.push_back(key);
                passed_moves = false;
                passed_all = false;
                continue;
            }

            int tmp = static_cast<int>(expected.get_move(key) - result.get_move(key));
            evaluated_moves[key] = tmp;
            if ( tmp ) {
                passed_moves = false;
                passed_all = false;
            }
        }

        for ( const auto& [key, value] : result.move_data ) {
            if ( expected.move_data.count(key) == 0 ) {
                extra_moves.push_back(key);
                passed_moves = false;
                passed_all = false;
            }
        }
    }
}

void DepthTest::print_test() const
{
    if ( passed_all ) {
        return;
    }

    std::cout << THIN_LINE << '\n'
        << "DEPTH: " << depth << '\n'
        << DOTTED_LINE << '\n';

    print_table_header();

    if ( passed_detailed ) {
        std::cout << "Passed detailed\n";
    }
    else {
        for ( const auto& [name, diff] : evaluated_detailed ) {
            if ( skip_key(name) ) continue;

            if ( diff ) {
                print_failed_row(name, result.get_detailed(name), expected.get_detailed(name), diff);
            }
            else {
                print_passed_row(name, result.get_detailed(name));
            }
        }
    }


    if ( has_move_data ) {
        std::cout << DOTTED_LINE << '\n';
        if ( passed_moves ) {
            std::cout << "PASSED MOVES -> TODO, MAKE LOOK NICE LOL\n";
            return;
        }

        for ( const auto& [name, diff] : evaluated_moves ) {
            if ( diff != 0 ) {
                print_failed_row(name, result.get_move(name), expected.get_move(name), diff);
            }
        }

        for ( const auto& move : missing_moves ) {
            std::cout << "Missing: " << move << '\n';
        }

        for ( const auto& move : extra_moves ) {
            std::cout << "Too much: " << move << '\n';
        }
    }


    return;


    if ( has_move_data ) {
        std::cout << DOTTED_LINE << '\n';
        if ( passed_moves ) {
            for ( const auto& [name, diff] : evaluated_moves ) {
                if ( diff ) {
                    print_failed_row(name, result.get_move(name), expected.get_move(name), diff);
                }
                else {
                    // print_passed_row(name, result.get_move(name));
                }
            }
        }
        else {
            std::cout << "Failed moves\n";
            if ( missing_moves.empty() ) return;

            std::cout << "Missing those moves: \n";
            for ( const auto& move : missing_moves ) {
                std::cout << move << ", ";
            }
            std::cout << '\n';
        }
    }
}

// ==========================================
//  POSITIONTEST
// ==========================================
PositionTest::PositionTest(const json test_data) : data(test_data), fen(data["FEN"])
{
    for ( const auto& item : data["depths"].items() ) {
        depths.emplace_back(fen, item.key(), item.value());
    }
}

void PositionTest::eval()
{
    for ( const auto& test : depths ) {
        if ( !test.passed_all ) passed_all = false;
    }
}

void PositionTest::run_tests()
{
    for ( auto& t : depths ) {
        t.run_test();
    }

    eval();
}

void PositionTest::print_position_header() const
{
    std::cout << FAT_LINE << '\n'
        << std::left << std::setw(COL_SPACING) << "FEN: "
        << std::left << std::setw(COL_SPACING) << fen
        << '\n';
}

void PositionTest::print() const
{
    print_position_header();

    if ( passed_all ) {
        std::stringstream ss;
        ss << depths.size() << "/" << depths.size();
        std::cout << GREEN
            << std::left << std::setw(COL_SPACING) << "Passed:"
            << std::left << std::setw(COL_SPACING) << ss.str()
            << RESET << '\n';
        return;
    }

    unsigned passed = 0;
    for ( const auto& test : depths ) {
        if ( test.passed_all ) ++passed;
    }

    std::stringstream ss;
    ss << (depths.size() - passed) << "/" << depths.size();
    std::cout << RED
        << std::left << std::setw(COL_SPACING) << "Failed:"
        << std::left << std::setw(COL_SPACING) << ss.str()
        << RESET << '\n';

    for ( const auto& test : depths ) {
        test.print_test();
    }
}

// ==========================================
//  TESTSUITE
// ==========================================

PerftTestSuite::PerftTestSuite(const std::string& json_path)
{
    std::ifstream file(json_path);

    if ( !file ) {
        const std::string error = "Failed to open \'" + json_path + "\'. Check if the file exists and the path is correct.\n";
        LOG_ERROR << error;
        throw std::runtime_error(error);
    }

    json data;

    try {
        file >> data;
    }
    catch ( const json::parse_error& e ) {
        const std::string error = "Parse error: " + std::string(e.what()) + "\nCheck if the file \'" + json_path + "\' is empty or contains valid JSON.\n";
        LOG_ERROR << error;
        throw std::runtime_error(error);
    }

    for ( const auto& item : data["perft_tests"] ) {
        tests.push_back({ item });
    }
}


void PerftTestSuite::runTests()
{
    auto begin = std::chrono::high_resolution_clock::now();
    unsigned passed = 0;

    for ( auto& t : tests ) {
        t.run_tests();
        if ( t.passed_all ) ++passed;
        else t.print();
    }
    auto end = std::chrono::high_resolution_clock::now();

    // for ( auto& t : tests ) {  if ( t.passed_all ) t.print(); }

    std::cout << FAT_LINE << '\n'
        << "PERFT TEST OVERVIEW:\n"
        << DOTTED_LINE << '\n';

    if ( passed == tests.size() ) {
        std::stringstream ss;
        ss << passed << '/' << tests.size();
        std::cout << GREEN
            << std::left << std::setw(COL_SPACING) << "Passed:"
            << std::left << std::setw(COL_SPACING) << ss.str()
            << RESET << '\n';
    }
    else {
        std::stringstream ss;
        ss << (tests.size() - passed) << '/' << tests.size();
        std::cout << RED
            << std::left << std::setw(COL_SPACING) << "Failed:"
            << std::left << std::setw(COL_SPACING) << ss.str()
            << RESET << '\n';
    }

    std::stringstream ss;
    ss << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms";
    std::cout << std::left << std::setw(COL_SPACING) << "Took: "
        << std::left << std::setw(COL_SPACING) << ss.str() << '\n'
        << FAT_LINE << '\n';
}
