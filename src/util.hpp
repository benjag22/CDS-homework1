#pragma once

#include <fstream>
#include <string>
#include <sdsl/suffix_arrays.hpp>

inline std::string do_bwt(std::string &text) {
    if (text[text.size() - 1] != '\0') {
        text += '\0';
    }

    const int64_t n = text.size(); // NOLINT(*-narrowing-conversions)
    sdsl::int_vector sa(1, 0, sdsl::bits::hi(n) + 1);
    sa.resize(n);
    sdsl::algorithm::calculate_sa(reinterpret_cast<const unsigned char *>(text.data()), n, sa);

    std::string bwt(n, 0);
    const int64_t to_add[2] = {-1, n - 1};

    for (int64_t i = 0; i < n; i++) {
        bwt[i] = text[sa[i] + to_add[sa[i] == 0]];
    }

    return bwt;
}

inline std::string read_file(const std::string &file_path) {
    std::ifstream file(file_path);
    if (!file.is_open() || !file.good() || file.bad() || file.fail() || file.eof()) {
        std::cerr << "Error: Could not open file: " << file_path << std::endl;
        std::exit(EXIT_FAILURE);
    }

    file.seekg(0, std::ios::end);
    const int64_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string contents(file_size, 0);
    file.read(contents.data(), file_size);
    file.close();

    return contents;
}
