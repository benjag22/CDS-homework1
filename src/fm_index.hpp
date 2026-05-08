#pragma once

#include <cmath>
#include <filesystem>
#include <string>
#include <sdsl/wavelet_trees.hpp>

#include "util.hpp"
#include "wavelet_tree.hpp"

// Enumeration of wavelet tree implementations to use for rank queries.
enum class WTType {
    // Custom wavelet tree implementation.
    OWN_IMPL,
    // SDSL integer-based wavelet tree.
    SDSL_INT,
    // SDSL Huffman-coded wavelet tree.
    SDSL_HUFF,
    // Brute force counting (for testing).
    BRUTE_FORCE,
};

// Data structure for fast pattern counting.
class fm_index {
    // The BWT (Burrows-Wheeler Transform) of the original text.
    std::string m_bwt;
    // Custom wavelet tree implementation for the BWT.
    wavelet_tree m_own_wt;
    // SDSL integer-based wavelet tree for the BWT.
    sdsl::wt_int<> m_sdsl_wt_int;
    // SDSL Huffman-coded wavelet tree for the BWT.
    sdsl::wt_huff<> m_sdsl_wt_huff;
    // Dictionary of unique characters in the BWT, sorted by ASCII order.
    std::vector<uint8_t> m_dict;
    // Inverse mapping from character to its index in the dictionary.
    std::unordered_map<uint8_t, uint8_t> m_dict_inv;
    // Count of characters lexicographically strictly less than each character in the BWT.
    std::unordered_map<uint8_t, uint64_t> m_lex_less_counts;

public:
    /**
     * Create an FM-index from a text file.
     * Constructs the BWT and builds all wavelet tree implementations for querying.
     * @param file_path Path to the text file.
     */
    explicit fm_index(const std::filesystem::path &file_path) {
        std::string text = read_file(file_path);
        m_bwt = do_bwt(text);
        m_own_wt.build(m_bwt);

        const std::string bwt_path = sdsl::ram_file_name(file_path.filename());
        sdsl::store_to_file(m_bwt, bwt_path);
        sdsl::construct(m_sdsl_wt_int, bwt_path, 1);
        sdsl::construct(m_sdsl_wt_huff, bwt_path, 1);
        sdsl::remove(bwt_path);

        m_dict = m_own_wt.dict();
        m_dict_inv = m_own_wt.dict_inv();
        m_lex_less_counts.reserve(m_dict.size());
        m_lex_less_counts[m_dict[0]] = 0;

        for (uint64_t i = 1; i < m_dict.size(); i++) {
            const uint8_t curr = m_dict[i];
            const uint8_t prev = m_dict[i - 1];
            m_lex_less_counts[curr] = m_lex_less_counts[prev] + m_own_wt.rank(m_own_wt.size() - 1, prev);
        }
    }

    /**
     * Count the number of occurrences of a pattern in the indexed text.
     * @param pattern The pattern to search for.
     * @param type The wavelet tree implementation to use for rank queries.
     * @return The number of times the pattern appears in the original text.
     */
    [[nodiscard]] uint64_t count(const std::string &pattern, const WTType type) const {
        uint64_t i = pattern.size() - 1;
        uint8_t c = pattern[i];

        if (!m_lex_less_counts.contains(c)) {
            return 0;
        }

        const uint8_t next_v = next_symbol(c);

        uint64_t start = m_lex_less_counts.at(c);
        uint64_t end = next_v >= c ? m_lex_less_counts.at(next_v) - 1 : m_bwt.size() - 1;

        // binary search on BWT
        while (start <= end && i > 0) {
            c = pattern[--i];

            if (!m_lex_less_counts.contains(c)) {
                return 0;
            }

            const uint64_t c_less_count = m_lex_less_counts.at(c);
            start = c_less_count + occ(c, start - 1, type);
            end = c_less_count + occ(c, end, type) - 1;
        }

        return end >= start ? end - start + 1 : 0;
    }

private:
    /**
     * Get the number of occurrences of a symbol up to position k.
     * Uses the specified wavelet tree implementation.
     * @param v The symbol to count.
     * @param k The position to count up to.
     * @param type The wavelet tree implementation to use.
     * @return The number of occurrences of the symbol up to position k.
     */
    [[nodiscard]] uint64_t occ(const uint8_t v, const uint64_t k, const WTType type) const {
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

    /**
     * Get the next symbol in the sorted dictionary (wrapping around).
     * @param symbol The current symbol.
     * @return The next symbol in sorted order, or the first symbol if at the end.
     * @throws std::out_of_range when CHECK_RANGES is set and symbol is not in dictionary.
     */
    [[nodiscard]] uint8_t next_symbol(const uint8_t symbol) const {
        if (CHECK_RANGES && !m_dict_inv.contains(symbol)) {
            throw std::out_of_range("symbol not in dictionary");
        }

        const uint8_t pos = m_dict_inv.at(symbol);
        const uint8_t next_pos = (pos + 1) % m_dict.size();
        return m_dict[next_pos];
    }

    /**
     * Count occurrences of a symbol using brute force (for testing).
     * @param k The position to count up to.
     * @param v The symbol to count.
     * @return The number of times symbol appears up to position k.
     */
    [[nodiscard]] uint64_t force_brute(const uint64_t k, const uint8_t v) const {
        const uint64_t limit = std::min(m_bwt.size(), k + 1);
        uint64_t counter = 0;

        for (uint64_t i = 0; i < limit; i++)
            if (m_bwt[i] == v)
                counter++;

        return counter;
    }
};
