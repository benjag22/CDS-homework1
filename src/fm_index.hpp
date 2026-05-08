#pragma once

#include <sdsl/wavelet_trees.hpp>

#include "wavelet_tree.hpp"

enum class WaveletTreeType {
    OWN_IMPLEMENTATION,
    HUFFMAN,
    INTEGER,
    BRUTE_FORCE
};

class fm_index {
    wavelet_tree m_own_tree;
    sdsl::wt_int<> m_integer_wt;
    sdsl::wt_huff<> m_huffman_wt;
    std::string m_text;

public:
    explicit fm_index(const std::string &text)
        : m_own_tree(build_own_implementation(text)),
          m_integer_wt(build_integer(text)), m_huffman_wt(build_huffman(text)),
          m_text(text) {
    }

    [[nodiscard]] uint32_t occ(const char v, const int k, const WaveletTreeType type) const {
        switch (type) {
            case WaveletTreeType::OWN_IMPLEMENTATION:
                return m_own_tree.rank(k, v);
            case WaveletTreeType::HUFFMAN:
                return m_huffman_wt.rank(k, v);
            case WaveletTreeType::INTEGER:
                return m_integer_wt.rank(k, v);
            case WaveletTreeType::BRUTE_FORCE:
                return force_brute(v, k);
        }
        return 0;
    }

private:
    [[nodiscard]] static wavelet_tree build_own_implementation(const std::string &text) {
        return wavelet_tree(text);
    }

    [[nodiscard]] static sdsl::wt_huff<> build_huffman(const std::string &text) {
        sdsl::wt_huff<> wt;
        sdsl::construct(wt, text);
        return wt;
    }

    [[nodiscard]] static sdsl::wt_int<> build_integer(const std::string &text) {
        sdsl::wt_int<> wt;
        sdsl::construct(wt, text);
        return wt;
    }

    [[nodiscard]] uint32_t force_brute(const char v, const int k) const {
        int counter = 0;
        for (int i = 0; i <= k && i < static_cast<int>(m_text.size()); i++) {
            if (m_text[i] == v) counter++;
        }
        return counter;
    }
};
