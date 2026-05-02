#pragma once
#include <vector>
#include <sdsl/wavelet_trees.hpp>
#include "wavelet_tree.hpp"

enum class WaveletTreeType {
    OWN_IMPLEMENTATION,
    HUFFMAN,
    INTEGER,
    BRUTE_FORCE
};

class FMIndex {
public:
    FMIndex(const std::string& text) : text(text) {
        own_tree = build_own_implementation(text);
        huffman_wt = build_huffman(text);
        integer_wt = build_integer(text);
    }

    [[nodiscard]] uint32_t occ(const char v, const int k, const WaveletTreeType type) const {
        switch (type) {
            case WaveletTreeType::OWN_IMPLEMENTATION:
                return own_tree.rank(k, v);
            case WaveletTreeType::HUFFMAN:
                return huffman_wt.rank(k, v);
            case WaveletTreeType::INTEGER:
                return integer_wt.rank(k, v);
            case WaveletTreeType::BRUTE_FORCE:
                return force_brute(v, k);
        }
        return 0;
    }

    WaveletTree own_tree;
    sdsl::wt_int<sdsl::rrr_vector<>> integer_wt;
    sdsl::wt_huff<> huffman_wt;
    std::string text;

private:
    static WaveletTree build_own_implementation(const std::string& text) {
        std::vector<uint32_t> sequence;
        for (const char c : text) {
            sequence.push_back(static_cast<uint32_t>(static_cast<int>(c)));
        }
        return WaveletTree(sequence);
    }

    static sdsl::wt_huff<> build_huffman(const std::string& text) {
        sdsl::wt_huff<> wt;
        sdsl::construct(wt, text);
        return wt;
    }

    static sdsl::wt_int<sdsl::rrr_vector<>> build_integer(const std::string& text) {
        sdsl::wt_int<sdsl::rrr_vector<>> wt;
        sdsl::construct(wt, text);
        return wt;
    }

    [[nodiscard]] uint32_t force_brute(const char v, const int k) const {
        int counter = 0;
        for (int i = 0; i <= k && i < static_cast<int>(text.size()); i++) {
            if (text[i] == v) counter++;
        }
        return counter;
    }
};