/**
 * Copyright @leonardlover on GitHub
 * https://github.com/leonardlover/uhr
 * Modified for this project
 */

#pragma once

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

static constexpr int MIN_RUNS = 32;

inline void validate_input(const uint64_t runs, const uint64_t lower, const uint64_t upper, const uint64_t step) {
    using std::cerr, std::endl, std::exit;

    if (runs < MIN_RUNS) {
        cerr << "<RUNS> must be at least " << MIN_RUNS << '.' << endl;
        exit(EXIT_FAILURE);
    }

    if (step == 0 || lower == 0 || upper == 0) {
        cerr << "<STEP>, <LOWER> and <UPPER> have to be positive." << endl;
        exit(EXIT_FAILURE);
    }

    if (lower > upper) {
        cerr << "<LOWER> must be at most equal to <UPPER>." << endl;
        exit(EXIT_FAILURE);
    }
}

inline void display_progress(const uint64_t runs, const uint64_t total) {
    using std::cout;

    const double progress = runs / static_cast<double>(total);
    constexpr uint64_t width = 70;
    const size_t p = width * progress;
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2) << progress * 100;
    const std::string &progress_string = stream.str();

    cout << "[";
    for (size_t i = 0; i < width; i++) {
        if (i < p)
            cout << '=';
        else if (i == p)
            cout << '>';
        else
            cout << ' ';
    }

    cout << "] " << progress_string << "%\r";
    cout.flush();
}

inline void quartiles(std::vector<double> &data, std::vector<double> &q) {
    q.resize(5);
    const size_t n = data.size();
    size_t p;

    std::ranges::sort(data);

    if (n < 4) {
        std::cerr << "quartiles needs at least 4 data points." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Get min and max
    q[0] = data.front();
    q[4] = data.back();

    // Find median
    if (n % 2 == 1) {
        q[2] = data[n / 2];
    } else {
        p = n / 2;
        q[2] = (data[p - 1] + data[p]) / 2.0;
    }

    // Find lower and upper quartiles
    if (n % 4 >= 2) {
        q[1] = data[n / 4];
        q[3] = data[3 * n / 4];
    } else {
        p = n / 4;
        q[1] = 0.25 * data[p - 1] + 0.75 * data[p];
        p = 3 * n / 4;
        q[3] = 0.75 * data[p - 1] + 0.25 * data[p];
    }
}
