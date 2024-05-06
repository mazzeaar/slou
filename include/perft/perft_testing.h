#pragma once

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <exception>
#include <chrono>
#include <iomanip>

#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "log.h"
#include "perft.h"
#include "config.h"
#include "zobrist.h"

#if SIMPLE_TEST == 1
inline bool skip_key(const std::string& name)
{
  return (name == "checkmates")
    || (name == "checks")
    || (name == "discovery_checks")
    || (name == "double_checks");
}
#elif
inline bool skip_key(const std::string& name) { return false; }
#endif

void print_table_header();
void print_failed_row(const std::string& name, u64 got, u64 expected, u64 difference);
void print_passed_row(const std::string& name, u64 got);

// ==========================================
//  DATASTRUCTURES FOR TESTS
// ==========================================
/**
 * @brief This class stores, runs and evaluates Perft results.
 * It receives a json with expected data from the PositionTest class.
 *
 * Depth test runs, evaluates and prints the perft tests.
 *
 * @see PositionTest
 * @see PerftTestSuite
 */
struct DepthTest {
  const std::string fen;
  int depth;
  json detailed;
  bool has_move_data = false;
  json moves;

  PerftResult result;
  PerftResult expected;

  // assuming the error is never over 2bn moves :)
  std::map<std::string, int> evaluated_moves;
  std::map<std::string, int> evaluated_detailed;

  std::vector<std::string> missing_moves;
  std::vector<std::string> extra_moves;

  bool passed_moves = true;
  bool passed_detailed = true;
  bool passed_all = true;

  DepthTest(const std::string& fen, const std::string& depth, json data);

  void run_test();
  void eval();
  void print_test() const;
};

/**
 * @brief This class is responsible for running perft tests for a position.
 * The json stores multiple positions with multiple depths each.
 * Only prints the failed tests.
 *
 * @see DepthTest
 * @see PerftTestSuite
 */
struct PositionTest {
  json data;
  const std::string fen;
  std::vector<DepthTest> depths;
  bool passed_all = true;

  PositionTest(const json test_data);

  void eval();
  void run_tests();
  void print_position_header() const;
  void print() const;
};

/**
 * @brief Responsible for managing the PositionTests.
 * Runs the perft tests for each starting position and directly prints the result.
 *
 * @see DepthTest
 * @see PositionTest
 */
class PerftTestSuite {
private:
  std::vector<PositionTest> tests;

public:
  PerftTestSuite(const std::string& json_path = "../data/perft_data.json");
  void runTests();
};
