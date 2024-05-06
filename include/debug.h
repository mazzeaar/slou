#pragma once

#include "config.h"

#include <map>
#include <chrono>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

struct BenchmarkStats {
    std::chrono::high_resolution_clock::duration total_duration;
    size_t n_calls;

    template <typename Units = std::chrono::nanoseconds>
    inline unsigned long long getAverage() const
    {
        return std::chrono::duration_cast<Units>(total_duration).count() / n_calls;
    }

    template <typename Units = std::chrono::nanoseconds>
    inline unsigned long long getTotal() const
    {
        return std::chrono::duration_cast<Units>(total_duration).count() / n_calls;
    }
};

extern std::map<std::string, BenchmarkStats> BENCHMARK_LIST;

#define DEBUG_INIT              std::map<std::string, BenchmarkStats> BENCHMARK_LIST;
#define FUNCTION_SIGNATURE      __FUNCTION__

#define DEBUG_START             auto start_##FUNCTION_SIGNATURE = std::chrono::high_resolution_clock::now();

#define DEBUG_END \
    do { \
        auto end_##FUNCTION_SIGNATURE = std::chrono::high_resolution_clock::now(); \
        auto duration_##FUNCTION_SIGNATURE = end_##FUNCTION_SIGNATURE - start_##FUNCTION_SIGNATURE; \
        BENCHMARK_LIST[FUNCTION_SIGNATURE].total_duration += duration_##FUNCTION_SIGNATURE; \
        BENCHMARK_LIST[FUNCTION_SIGNATURE].n_calls++; \
    } while(0)

inline std::map<std::string, BenchmarkStats> loadBenchmarkData(const std::string& filename)
{
    std::ifstream i(filename);

    if ( !i.good() || i.peek() == std::ifstream::traits_type::eof() ) {
        return {};
    }

    json j;
    i >> j;

    std::map<std::string, BenchmarkStats> historicalData;

    for ( auto& element : j.items() ) {
        BenchmarkStats stats;
        stats.total_duration = std::chrono::nanoseconds(element.value()["total_duration"]);
        stats.n_calls = element.value()["n_calls"];
        historicalData[element.key()] = stats;
    }

    return historicalData;
}

inline void saveBenchmarkData(const std::string& filename, const std::map<std::string, BenchmarkStats>& currentData, const std::map<std::string, BenchmarkStats>& historicalData)
{
    json j;

    // load existing data
    std::ifstream inputFile(filename);
    if ( inputFile.good() && inputFile.peek() != std::ifstream::traits_type::eof() ) {
        inputFile >> j;
    }

    for ( const auto& [key, currentStats] : currentData ) {
        auto it = historicalData.find(key);
        bool shouldUpdate = true;  // assume update if we dont have data

        // if data exists, compare averages
        if ( it != historicalData.end() ) {
            auto currentAvg = currentStats.getAverage();
            auto historicalAvg = it->second.getAverage();
            shouldUpdate = currentAvg < historicalAvg;  // only update if improvement
        }

        // update if neccessary
        if ( shouldUpdate ) {
            j[key] = {
                {"total_duration", std::chrono::duration_cast<std::chrono::nanoseconds>(currentStats.total_duration).count()},
                {"n_calls", currentStats.n_calls}
            };
        }
    }

    std::ofstream outputFile(filename);
    if ( !j.empty() ) {
        outputFile << std::setw(4) << j << std::endl;
    }
}


inline void print_benchmark_header()
{
    std::cout << "Benchmark Information:\n"
        << THIN_LINE << '\n'
        << std::left << std::setw(COL_SPACING) << "Function"
        << std::right << std::setw(COL_SPACING) << "Call Count"
        << std::right << std::setw(COL_SPACING) << "Total Time (ms)"
        << std::right << std::setw(COL_SPACING) << "Avg Time (ns)"
        << std::right << std::setw(COL_SPACING) << "Improvement (%)" << '\n'
        << DOTTED_LINE << '\n';
}

inline void print_benchmark(const std::string& name, const BenchmarkStats& cur_stats, const BenchmarkStats& best_stats)
{
    const auto cur_total_time = cur_stats.getTotal();
    const auto prev_avg = best_stats.getAverage();
    const auto cur_avg = cur_stats.getAverage();

    std::cout << std::left << std::setw(COL_SPACING) << name
        << std::right << std::setw(COL_SPACING) << cur_stats.n_calls
        << std::right << std::setw(COL_SPACING) << cur_total_time
        << std::right << std::setw(COL_SPACING) << cur_avg;

    const int improvement = 100 - (cur_avg * 100 / prev_avg);

    if ( improvement == 0 ) {
        std::cout << std::right << std::setw(COL_SPACING) << improvement << '\n';
    }
    else if ( improvement > 0 ) {
        std::cout << GREEN
            << std::right << std::setw(COL_SPACING) << improvement
            << RESET << '\n';
    }
    else {
        std::cout << RED
            << std::right << std::setw(COL_SPACING) << improvement
            << RESET << '\n';
    }
}

inline std::vector<std::pair<std::string, BenchmarkStats>> sort_benchmark(const std::map<std::string, BenchmarkStats>& data)
{
    std::vector<std::pair<std::string, BenchmarkStats>> sorted_list(data.begin(), data.end());

    std::sort(sorted_list.begin(), sorted_list.end(), [&] (const auto& a, const auto& b)
    {
        auto avg_a = std::chrono::duration_cast<std::chrono::nanoseconds>(a.second.total_duration).count() / a.second.n_calls;
        auto avg_b = std::chrono::duration_cast<std::chrono::nanoseconds>(b.second.total_duration).count() / b.second.n_calls;
        return avg_a > avg_b;
    });

    return sorted_list;
}

inline void printBenchmarkInfo()
{
    const std::string historical_data_path = "../data/historical_data.json";

    print_benchmark_header();

    auto sorted_list = sort_benchmark(BENCHMARK_LIST);
    auto historical_data = loadBenchmarkData(historical_data_path);

    for ( const auto& [name, currentStats] : sorted_list ) {
        const auto& prev_stats = historical_data.find(name);
        print_benchmark(name, currentStats, prev_stats->second);
    }

    try {
        saveBenchmarkData(historical_data_path, BENCHMARK_LIST, historical_data);
    }
    catch ( const json::parse_error& e ) {
        std::cout << "here!\n";
    }

    std::cout << FAT_LINE << '\n';
}
