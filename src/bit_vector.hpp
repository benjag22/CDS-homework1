#pragma once

#include <cstdint>
#include <limits>
#include <vector>

// Gets the number of bits set to 1 in a 64-bit integer.
#define POPCNT(x) __builtin_popcountll(x)

// Data structure that supports constant time rank operations with bit vectors of arbitrary size.
class bit_vector {
    // The size of the super blocks in bits.
    static constexpr uint64_t S = 512;
    // The size of the blocks in bits.
    static constexpr uint64_t B = 128;
    // Integer with all bits set to 1.
    static constexpr uint64_t ONE = std::numeric_limits<uint64_t>::max();

    // The size of the bit_vector in bits.
    uint64_t m_size;
    // The bit array of size ceil(size() / 64).
    std::vector<uint64_t> m_bit_array;
    // The number of 1s up to each block, relative to its super block.
    std::vector<uint16_t> m_blocks;
    // The number of 1s up to each super block.
    std::vector<uint64_t> m_super_blocks;

public:
    /**
     * Create a bit_vector with an initial size and default bit.
     * @param size Number of bits this bit_vector will hold.
     * @param default_value Default value for the initial bits.
     */
    explicit bit_vector(const uint64_t size = 0, const uint8_t default_value = 0)
        : m_size(size),
          m_bit_array((size + 63) / 64, default_value == 0 ? 0 : ONE),
          m_blocks((size + B - 1) / B, 0),
          m_super_blocks((size + S - 1) / S, 0) {
    }

    /**
     * Create a bit_vector from a bit array.
     * Bits are read from least-significant (lsb) to most-significant (msb).
     * @param bit_array The bit array this bit_vector will hold.
     */
    explicit bit_vector(const std::vector<uint64_t> &bit_array)
        : m_size(bit_array.size() * 64),
          m_bit_array(bit_array),
          m_blocks((m_size + B - 1) / B, 0),
          m_super_blocks((m_size + S - 1) / S, 0) {
        build_rank();
    }

    /**
     * Build the helper structures that allow for a constant time rank operation.
     * This is required if you didn't create this bit_vector from a bit array.
     */
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

    /**
     * Set the size in bits of this bit_vector and set the bits to a default value.
     * You must call build_rank() later if you want to use rank operations.
     * @param size The new size of the bit_vector.
     * @param default_value Optional default value for the bits.
     */
    void set_size(const uint64_t size, const uint64_t default_value = 0) {
        m_size = size;
        m_bit_array.assign((size + 63) / 64, default_value == 0 ? 0 : ONE);
        m_blocks.assign((size + B - 1) / B, 0);
        m_super_blocks.assign((size + S - 1) / S, 0);
    }

    /**
     * Get the size of the bit_vector in bits.
     * @return The number of bits this bit_vector holds. May not be word multiple.
     */
    [[nodiscard]] uint64_t size() const {
        return m_size;
    }

    /**
     * Access the word at the i-th position.
     * There's a total of ceil(size() / 64) words in a bit_vector.
     * Bits are read from least-significant (lsb) to most-significant (msb).
     * @param i The index to access.
     * @return The word at the i-th position.
     */
    uint64_t &operator[](const uint64_t i) {
        return m_bit_array[i];
    }

    /**
     * Flip a bit at the i-th position.
     * Bits are written from least-significant (lsb) to most-significant (msb).
     * You must call build_rank() afterward if you want to use rank operations.
     * @param i The i-th bit to flip.
     */
    void flip(const uint64_t i) {
        m_bit_array[i / 64] ^= 1ull << (i % 64);
    }

    /**
     * Access the bit at the i-th position.
     * There's a total of size() bits in a bit_vector.
     * Bits are read from least-significant (lsb) to most-significant (msb).
     * @param i The i-th bit to access.
     * @return The value of the i-th bit.
     */
    [[nodiscard]] uint8_t access(const uint64_t i) const {
        return m_bit_array[i / 64] >> (i % 64) & 1;
    }

    /**
     * Get the number of 1s up to the index (inclusive).
     * @param i The position to count up to.
     * @return The number of 1s up to the index.
     */
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

    /**
     * Get the number of 1s between both indices (inclusive).
     * @param i The position to count from.
     * @param j The position to count up to.
     * @return The number of 1s in the range.
     */
    [[nodiscard]] uint64_t rank_1(const uint64_t i, const uint64_t j) const {
        return rank_1(j) - rank_1(i - 1);
    }

    /**
     * Get the number of 0s up to the index (inclusive).
     * @param i The position to count up to.
     * @return The number of 0s up to the index.
     */
    [[nodiscard]] uint64_t rank_0(const uint64_t i) const {
        return i + 1 - rank_1(i);
    }

    /**
     * Get the number of 0s between both indices (inclusive).
     * @param i The position to count from.
     * @param j The position to count up to.
     * @return The number of 0s in the range.
     */
    [[nodiscard]] uint64_t rank_0(const uint64_t i, const uint64_t j) const {
        return rank_0(j) - rank_0(i - 1);
    }
};
