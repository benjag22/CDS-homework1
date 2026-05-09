/**
 * Copyright @leonardlover on GitHub
 * https://github.com/leonardlover/uhr
 * Modified for this project
 */

#pragma once

#include <array>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "uhr_utils.hpp"
#include "../fm_index.hpp"
#include "../util.hpp"

/**
 * Run performance measurements.
 * @param input_file_path Path of the file where test data will be read.
 * @param results_file_path Path of the file where performance data will be written.
 * @param runs Number of runs per test case. Should be >= 32.
 * @param lower Range of test cases. Should be > 0.
 * @param upper Range of test cases. Should be > 0.
 * @param step Step of test cases. Should be > 0.
 * @param mult_step Whether to use multiplicative steps instead of additive.
 */
inline void uhr(
    const std::string &input_file_path,
    const std::string &results_file_path,
    const uint64_t runs,
    const uint64_t lower,
    const uint64_t upper,
    const uint64_t step,
    const bool mult_step
) {
    validate_input(runs, lower, upper, step);

    constexpr std::array wt_types{
        WTType::OWN_IMPL,
        WTType::SDSL_INT,
        WTType::SDSL_HUFF,
        WTType::BRUTE_FORCE,
    };

    const uint64_t runs_multiplicative = runs
        * (static_cast<uint64_t>(std::log(upper / static_cast<double>(lower)) / std::log(step)) + 1);
    const uint64_t total_runs_additive = (wt_types.size() - 1)
        * (mult_step ? runs_multiplicative : runs * ((upper - lower) / step + 1))
        + runs_multiplicative;

    std::vector<double> times(runs);
    std::vector<double> q;
    std::chrono::duration<double, std::nano> elapsed_time{};

    std::cout <<
        "Current setup\n\n"
        "Input file:           " << input_file_path << "\n"
        "Output file:          " << results_file_path << "\n"
        "Runs per test:        " << runs << "\n"
        "Pattern length range: [" << lower << ", " << upper << "]\n"
        "Pattern length step:  " << step << (mult_step ? " (multiplicative)" : " (additive [mult. for b.f.])") << "\n"
        "Total runs:           " << total_runs_additive << "\n"
        << std::endl;

    // File to write time data
    std::ofstream time_data(results_file_path);
    time_data << "type,mem,n,t_mean,t_stdev,t_Q0,t_Q1,t_Q2,t_Q3,t_Q4\n";

    std::cout << "Building FM-Index..." << std::endl;

    const fm_index fmi(input_file_path);
    const std::string &text = fmi.text();

    if (upper > text.size() - 2) {
        std::cerr << "Upper bound is longer than the input text. Aborting tests." << std::endl;
        return;
    }

    int_generator<uint64_t> generator(0, text.size() - upper - 2);

    // Begin testing
    const std::string test_name = std::filesystem::path(results_file_path).stem().string();
    std::cout << "Running " << test_name << " tests...\n\n";
    uint64_t executed_runs = 0;

    for (const WTType &wt_type: wt_types) {
        const uint64_t memory = fmi.size_in_bytes(wt_type);
        const bool is_mult = mult_step || wt_type == WTType::BRUTE_FORCE;

        for (uint64_t n = lower; n <= upper; is_mult ? n *= step : n += step) {
            double mean_time = 0;
            double time_stdev = 0;

            // Test configuration goes here
            const uint64_t pattern_start = generator();
            const std::string pattern(text.begin() + pattern_start, text.begin() + pattern_start + n);

            // Run to compute elapsed time
            for (size_t i = 0; i < runs; i++) {
                // Remember to change total depending on step type
                display_progress(++executed_runs, total_runs_additive);

                auto begin_time = std::chrono::high_resolution_clock::now();
                // Function to test goes here
                // ReSharper disable once CppNoDiscardExpression
                fmi.count(pattern, wt_type);
                auto end_time = std::chrono::high_resolution_clock::now();

                elapsed_time = end_time - begin_time;
                times[i] = elapsed_time.count();

                mean_time += times[i];
            }

            // Compute statistics
            mean_time /= runs;

            for (size_t i = 0; i < runs; i++) {
                const double dev = times[i] - mean_time;
                time_stdev += dev * dev;
            }

            time_stdev /= runs - 1; // Subtract 1 to get unbiased estimator
            time_stdev = std::sqrt(time_stdev);

            quartiles(times, q);

            time_data
                << wt_type << ','
                << memory << ','
                << n << ','
                << mean_time << ','
                << time_stdev << ','
                << q[0] << ','
                << q[1] << ','
                << q[2] << ','
                << q[3] << ','
                << q[4] << '\n';
        }
    }

    // This is to keep loading bar after testing
    std::cout << "\n\n" << test_name << " done!" << std::endl;

    time_data.close();
}
