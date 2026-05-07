#pragma once

#include <cstdint>
#include <vector>

#define POPCNT(x) __builtin_popcountll(x)

class bit_vector {
    static constexpr uint64_t S = 512;
    static constexpr uint64_t B = 128;
    static constexpr uint64_t ONE = std::numeric_limits<uint64_t>::max();

    const uint64_t m_size;
    std::vector<uint64_t> m_bit_array;
    std::vector<uint16_t> m_blocks;
    std::vector<uint64_t> m_super_blocks;

public:
    explicit bit_vector(const uint64_t size, const uint8_t default_value = 0)
        : m_size(size),
          m_bit_array((size + 63) / 64, default_value == 0 ? 0 : ONE),
          m_blocks((size + B - 1) / B, 0),
          m_super_blocks((size + S - 1) / S, 0) {
    }

    explicit bit_vector(const std::vector<uint64_t> &bit_array)
        : m_size(bit_array.size() * 64),
          m_bit_array(bit_array),
          m_blocks((m_size + B - 1) / B, 0),
          m_super_blocks((m_size + S - 1) / S, 0) {
        build_rank();
    }

    void build_rank() {
        uint64_t si = 1, bi = 1, s_count = 0, b_count = 0, offset = 0;

        for (uint64_t const &num: m_bit_array) {
            const uint8_t count = POPCNT(num);

            offset += 64;
            s_count += count;
            b_count += count;

            if (offset % S == 0) {
                m_super_blocks[si++] = s_count;
                b_count = 0;
            }

            if (offset % B == 0) {
                m_blocks[bi++] = b_count;
            }
        }
    }

    [[nodiscard]] uint64_t size() const {
        return m_size;
    }

    void set(const uint64_t i) {
        m_bit_array[i / 64] |= 1 << (i % 64);
    }

    [[nodiscard]] uint8_t access(const uint64_t i) const {
        return m_bit_array[i / 64] >> (i % 64) & 1;
    }

    [[nodiscard]] uint64_t rank_1(const uint64_t i) const {
        const uint64_t s_count = m_super_blocks[i / S];
        const uint64_t b_count = m_blocks[i / B];
        const uint64_t bucket = i / 64;
        const uint64_t offset = 63 - i % 64;
        const uint64_t pop_count = POPCNT(m_bit_array[bucket] << offset);
        uint64_t count = s_count + b_count + pop_count;
        if (i % B >= 64) {
            count += POPCNT(m_bit_array[bucket - 1]);
        }
        return count;
    }

    [[nodiscard]] uint64_t rank_1(const uint64_t i, const uint64_t j) const {
        return rank_1(j) - rank_1(i - 1);
    }

    [[nodiscard]] uint64_t rank_0(const uint64_t i) const {
        return i + 1 - rank_1(i);
    }

    [[nodiscard]] uint64_t rank_0(const uint64_t i, const uint64_t j) const {
        return rank_0(j) - rank_0(i - 1);
    }
};
