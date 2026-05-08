#pragma once

#include <cmath>
#include <filesystem>
#include <string>
#include <sdsl/wavelet_trees.hpp>

#include "util.hpp"
#include "wavelet_tree.hpp"

enum class WTType {
    OWN_IMPL,
    SDSL_INT,
    SDSL_HUFF,
    BRUTE_FORCE,
};

class fm_index {
    std::string m_text;
    std::string m_bwt;
    wavelet_tree m_own_wt;
    sdsl::wt_int<> m_sdsl_wt_int;
    sdsl::wt_huff<> m_sdsl_wt_huff;

public:
    explicit fm_index(const std::filesystem::path &file_path) {
        m_text = read_file(file_path);
        m_bwt = do_bwt(m_text + '\0');
        m_own_wt.build(m_bwt);

        const std::string bwt_path = sdsl::ram_file_name(file_path.filename());
        sdsl::store_to_file(m_bwt, bwt_path);
        sdsl::construct(m_sdsl_wt_int, bwt_path, 1);
        sdsl::construct(m_sdsl_wt_huff, bwt_path, 1);
        sdsl::remove(bwt_path);

        std::cout <<
            "own_wt:  " << m_own_wt.size_in_bytes() << "\n"
            "wt_int:  " << sdsl::size_in_bytes(m_sdsl_wt_int) << "\n"
            "wt_huff: " << sdsl::size_in_bytes(m_sdsl_wt_huff) << "\n"
            << std::endl;
    }

    [[nodiscard]] uint64_t occ(const uint64_t k, const uint8_t v, const WTType type) const {
        switch (type) {
            case WTType::OWN_IMPL:
                return m_own_wt.rank(k, v);
            case WTType::SDSL_INT:
                return m_sdsl_wt_int.rank(k, v);
            case WTType::SDSL_HUFF:
                return m_sdsl_wt_huff.rank(k, v);
            case WTType::BRUTE_FORCE:
                return force_brute(k, v);
        }
        __builtin_unreachable();
    }

private:
    [[nodiscard]] uint64_t force_brute(const uint64_t k, const uint8_t v) const {
        const uint64_t limit = std::min(m_text.size(), k + 1);
        uint64_t counter = 0;

        for (uint64_t i = 0; i < limit; i++)
            if (m_text[i] == v)
                counter++;

        return counter;
    }
};
