#pragma once

#include <cstdint>
#include <limits>

#include "config.hpp"

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
    uint64_t m_size = 0;
    // The size of the bit array.
    uint64_t m_bit_array_size = 0;
    // The size of the blocks array.
    uint64_t m_blocks_size = 0;
    // The size of the super blocks array.
    uint64_t m_super_blocks_size = 0;
    // The bit array of size ceil(size() / 64).
    uint64_t *m_bit_array = nullptr;
    // The number of 1s up to each block, relative to its super block.
    uint16_t *m_blocks = nullptr;
    // The number of 1s up to each super block.
    uint64_t *m_super_blocks = nullptr;

public:
    /**
     * Create a bit_vector with an initial size and default bit.
     * @param size Number of bits this bit_vector will hold.
     * @param default_value Default value for the initial bits.
     */
    explicit bit_vector(const uint64_t size = 0, const uint8_t default_value = 0) {
        set_size(size, default_value);
    }

    ~bit_vector() {
        if (m_bit_array != nullptr) {
            delete[] m_bit_array;
        }
        if (m_blocks != nullptr) {
            delete[] m_blocks;
        }
        if (m_super_blocks != nullptr) {
            delete[] m_super_blocks;
        }
    }

    /**
     * Build the helper structures that allow for a constant time rank operation.
     * This is required if you didn't create this bit_vector from a bit array.
     */
    void build_rank() const {
        uint64_t si = 1, bi = 1, s_count = 0, offset = 0;
        uint16_t b_count = 0;

        for (uint64_t i = 0; i < m_bit_array_size; i++) {
            const uint64_t num = m_bit_array[i];
            const uint8_t count = POPCNT(num);

            offset += 64;
            s_count += count;
            b_count += count;

            // the compiler should be smart enough to optimize %
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
        if (m_bit_array != nullptr) {
            delete[] m_bit_array;
        }
        if (m_blocks != nullptr) {
            delete[] m_blocks;
        }
        if (m_super_blocks != nullptr) {
            delete[] m_super_blocks;
        }

        if (size == 0) {
            return;
        }

        m_size = size;
        m_bit_array_size = (size + 63) / 64;
        m_blocks_size = (size + B - 1) / B;
        m_super_blocks_size = (size + S - 1) / S;
        m_bit_array = new uint64_t[m_bit_array_size];
        m_blocks = new uint16_t[m_blocks_size];
        m_super_blocks = new uint64_t[m_super_blocks_size];

        for (uint64_t i = 0; i < m_bit_array_size; i++) {
            m_bit_array[i] = default_value;
        }
        for (uint64_t i = 0; i < m_blocks_size; i++) {
            m_blocks[i] = 0;
        }
        for (uint64_t i = 0; i < m_super_blocks_size; i++) {
            m_super_blocks[i] = 0;
        }
    }

    /**
     * Get the size of the bit_vector in bits.
     * @return The number of bits this bit_vector holds. May not be word multiple.
     */
    [[nodiscard]] uint64_t size() const {
        return m_size;
    }

    /**
     * Flip a bit at the i-th position.
     * Bits are written from least-significant (lsb) to most-significant (msb).
     * You must call build_rank() afterward if you want to use rank operations.
     * @param i The i-th bit to flip.
     * @throws std::out_of_range when CHECK_RANGES is set and i >= size()
     */
    void flip(const uint64_t i) {
        // the compiler should be smart enough to optimize / and %
        if (CHECK_RANGES && i >= m_size) {
            throw std::out_of_range("access out of range");
        }
        m_bit_array[i / 64] ^= 1ull << (i % 64);
    }

    /**
     * Access the bit at the i-th position.
     * There's a total of size() bits in a bit_vector.
     * Bits are read from least-significant (lsb) to most-significant (msb).
     * @param i The i-th bit to access.
     * @return The value of the i-th bit.
     * @throws std::out_of_range when CHECK_RANGES is set and i >= size()
     */
    [[nodiscard]] uint8_t access(const uint64_t i) const {
        // the compiler should be smart enough to optimize / and %
        if (CHECK_RANGES && i >= m_size) {
            throw std::out_of_range("access out of range");
        }
        return m_bit_array[i / 64] >> (i % 64) & 1;
    }

    /**
     * Get the number of 1s up to the index (inclusive).
     * @param i The position to count up to.
     * @return The number of 1s up to the index.
     * @throws std::out_of_range when CHECK_RANGES is set and i >= size()
     */
    [[nodiscard]] uint64_t rank_1(const uint64_t i) const {
        // the compiler should be smart enough to optimize / and %
        if (CHECK_RANGES && i >= m_size) {
            throw std::out_of_range("access out of range");
        }
        const uint64_t s_count = m_super_blocks[i / S];
        const uint64_t b_count = m_blocks[i / B];
        const uint64_t bucket = i / 64;
        const uint64_t offset = 63 - i % 64;
        const bool needs_prev = i % B >= 64;
        const uint64_t pop_count = POPCNT(m_bit_array[bucket] << offset);
        // trick for branchless
        const uint64_t prev = needs_prev * POPCNT(m_bit_array[bucket - needs_prev]);
        return s_count + b_count + pop_count + prev;
    }

    /**
     * Get the number of 1s between both indices (inclusive).
     * @param i The position to count from.
     * @param j The position to count up to.
     * @return The number of 1s in the range.
     * @throws std::out_of_range when CHECK_RANGES is set and i - 1 >= size() or j >= size()
     */
    [[nodiscard]] uint64_t rank_1(const uint64_t i, const uint64_t j) const {
        return rank_1(j) - rank_1(i - 1);
    }

    /**
     * Get the number of 0s up to the index (inclusive).
     * @param i The position to count up to.
     * @return The number of 0s up to the index.
     * @throws std::out_of_range when CHECK_RANGES is set and i >= size()
     */
    [[nodiscard]] uint64_t rank_0(const uint64_t i) const {
        return i + 1 - rank_1(i);
    }

    /**
     * Get the number of 0s between both indices (inclusive).
     * @param i The position to count from.
     * @param j The position to count up to.
     * @return The number of 0s in the range.
     * @throws std::out_of_range when CHECK_RANGES is set and i - 1 >= size() or j >= size()
     */
    [[nodiscard]] uint64_t rank_0(const uint64_t i, const uint64_t j) const {
        return rank_0(j) - rank_0(i - 1);
    }

    /**
     * Get the size of this bit_vector in bytes.
     * @return The size of this bit_vector in bytes.
     */
    [[nodiscard]] uint64_t size_in_bytes() const {
        return sizeof(bit_vector)
            + m_bit_array_size * sizeof(uint64_t)
            + m_blocks_size * sizeof(uint16_t)
            + m_super_blocks_size * sizeof(uint64_t);
    }
};
