#pragma once
#include <vector>
#include <cstdint>
#include "util.hpp"

class bitVector {
    static constexpr int BITS = 32; // bits for block
    std::vector<uint32_t> vec;      // blocks of 32 bits
    std::vector<uint32_t> count;    // prefix of 1's for block

public:
    bitVector() = default;

    explicit bitVector(const uint32_t n) {
        resize(n);
    }

    void resize(const uint32_t n) {
        const uint32_t blocks = (n + BITS - 1) / BITS;
        vec.assign(blocks, 0);
        count.assign(blocks, 0);
    }

    void set(const uint32_t i) {
        vec[i / BITS] |= (1u << (i % BITS));
    }

    void build_rank() {
        count[0] = 0;
        for (uint32_t i = 1; i < vec.size(); ++i) {
            count[i] = count[i - 1] + popcnt(vec[i - 1]);
        }
    }
    [[nodiscard]] uint32_t access(const uint32_t i) const {
        return (vec[i / BITS] >> (i % BITS)) & 1u;
    }

    [[nodiscard]] uint32_t rank_1(const uint32_t i) const {
        const uint32_t block = i / BITS;
        const uint32_t offset = i % BITS;
        const uint32_t mask = (offset == 31) ? 0xFFFFFFFF : (1u << (offset + 1)) - 1;
        return count[block] + popcnt(vec[block] & mask);
    }

    [[nodiscard]] uint32_t rank_1(const int i, const int j) const {
        return rank_1(j) - rank_1(i - 1);
    }

    [[nodiscard]] uint32_t rank_0(const int i) const {
        if (i < 0) return 0;
        return (i + 1) - rank_1(i);
    }

    [[nodiscard]] uint32_t rank_0(const int i, const int j) const {
        return rank_0(j) - rank_0(i - 1);
    }
};
