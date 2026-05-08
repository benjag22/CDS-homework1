#pragma once

#include <cstdint>
#include <set>
#include <stack>
#include <unordered_map>
#include <vector>

#include "bit_vector.hpp"
#include "config.hpp"

// Data structure that supports fast access and rank queries.
class wavelet_tree {
    // The sorted dictionary of unique characters in the text.
    std::vector<uint8_t> m_dict;
    // Inverse mapping from character to its index in the dictionary.
    std::unordered_map<uint8_t, uint8_t> m_dict_inv;
    // Bit vector storing the tree structure.
    bit_vector m_bit_vector;
    // The size of the original text.
    uint64_t m_size = 0;
    // The alphabet size.
    uint8_t m_sigma = 0;
    // The height of the tree.
    uint8_t m_height = 0;

public:
    /**
     * Create a default wavelet_tree.
     */
    wavelet_tree() = default;

    /**
     * Create and build a wavelet_tree from a text string.
     * @param text The text to build the wavelet tree from.
     */
    explicit wavelet_tree(const std::string &text) {
        build(text);
    }

    /**
     * Build a wavelet tree from a text string.
     * @param text The text to build the wavelet tree from.
     */
    void build(const std::string &text) {
        m_size = text.size();
        std::string copy = text;
        const std::set<uint8_t> chars(text.begin(), text.end());
        build_tree(copy, chars);
    }

    /**
     * Get the size of the text used to build this wavelet tree.
     * @return The number of characters in the original text.
     */
    [[nodiscard]] uint64_t size() const {
        return m_size;
    }

    /**
     * Get the height of this wavelet tree.
     * @return The height of the tree structure.
     */
    [[nodiscard]] uint64_t height() const {
        return m_height;
    }

    /**
     * Get the dictionary mapping characters to indices, sorted by ASCII order.
     * @return A const reference to the sorted character dictionary.
     */
    [[nodiscard]] const std::vector<uint8_t> &dict() const {
        return m_dict;
    }

    /**
     * Get the inverse dictionary mapping indices to characters.
     * @return A const reference to the inverse character mapping.
     */
    [[nodiscard]] const std::unordered_map<uint8_t, uint8_t> &dict_inv() const {
        return m_dict_inv;
    }

    /**
     * Get the underlying bit vector.
     * @return A const reference to the internal bit vector.
     */
    [[nodiscard]] const bit_vector &get_bit_vector() const {
        return m_bit_vector;
    }

    /**
     * Access the character at the i-th position in the original text.
     * @param i The position to access.
     * @return The character at the i-th position.
     * @throws std::out_of_range when CHECK_RANGES is set and i >= size()
     */
    [[nodiscard]] uint8_t access(uint64_t i) const {
        if (CHECK_RANGES && i >= m_size) {
            throw std::out_of_range("access out of range");
        }

        uint64_t offset = 0;
        uint64_t seq_size = m_size;
        uint8_t left = 0, right = m_sigma;
        uint8_t height = 0;

        // binary search through the tree levels
        while (right - left > 1) {
            const uint8_t pivot = left + (right - left + 1) / 2;
            const uint64_t base = m_size * height + offset;
            const uint64_t zeros_before_base = base > 0 ? m_bit_vector.rank_0(base - 1) : 0;
            const uint64_t total_zeros = m_bit_vector.rank_0(base + seq_size - 1) - zeros_before_base;

            if (m_bit_vector.access(base + i) == 0) {
                // is in the left
                i = m_bit_vector.rank_0(base + i) - zeros_before_base - 1;
                seq_size = total_zeros;
                right = pivot;
            } else {
                // is in the right
                const uint64_t ones_up_to_i = i + 1 - (m_bit_vector.rank_0(base + i) - zeros_before_base);
                i = ones_up_to_i - 1;
                offset += total_zeros;
                seq_size -= total_zeros;
                left = pivot;
            }

            height++;
        }

        return m_dict[left];
    }

    /**
     * Get the number of occurrences of a symbol up to position i (inclusive).
     * @param i The position to count up to.
     * @param symbol The symbol to count.
     * @return The number of times the symbol appears up to position i.
     * @throws std::out_of_range when CHECK_RANGES is set and i >= size()
     */
    [[nodiscard]] uint64_t rank(const uint64_t i, uint8_t const symbol) const {
        if (CHECK_RANGES && i >= m_size) {
            throw std::out_of_range("access out of range");
        }

        if (!m_dict_inv.contains(symbol)) {
            return 0;
        }

        const uint8_t si = m_dict_inv.at(symbol);
        uint64_t offset = 0;
        uint64_t seq_size = m_size;
        uint8_t left = 0, right = m_sigma;
        uint8_t height = 0;
        uint64_t pos = i;

        // binary search through the tree levels
        while (right - left > 1) {
            const uint8_t pivot = left + (right - left + 1) / 2;
            const uint64_t base = m_size * height + offset;
            const uint64_t zeros_before_base = base > 0 ? m_bit_vector.rank_0(base - 1) : 0;
            const uint64_t total_zeros = m_bit_vector.rank_0(base + seq_size - 1) - zeros_before_base;
            const uint64_t zeros_up_to_pos = m_bit_vector.rank_0(base + pos) - zeros_before_base;
            const uint64_t ones_up_to_pos = pos + 1 - zeros_up_to_pos;

            if (si < pivot) {
                // is in the left
                if (zeros_up_to_pos == 0) {
                    return 0;
                }

                pos = zeros_up_to_pos - 1;
                seq_size = total_zeros;
                right = pivot;
            } else {
                // is in the right
                if (ones_up_to_pos == 0) {
                    return 0;
                }

                pos = ones_up_to_pos - 1;
                offset += total_zeros;
                seq_size -= total_zeros;
                left = pivot;
            }

            height++;
        }

        return pos + 1;
    }

    /**
     * Get the size of this wavelet_tree in bytes.
     * @return The size of this wavelet_tree in bytes.
     */
    [[nodiscard]] uint64_t size_in_bytes() const {
        return sizeof(wavelet_tree)
            - sizeof(bit_vector)
            + m_bit_vector.size_in_bytes()
            + m_dict.size() * sizeof(uint8_t)
            + m_dict_inv.size() * (sizeof(uint8_t) + sizeof(uint8_t));
    }

    [[nodiscard]] bool operator==(const std::vector<uint64_t> &v) const {
        return v == m_bit_vector;
    }

private:
    /**
     * Build the wavelet tree structure from text and its character set.
     * Initializes the dictionary, inverse dictionary, and calculates tree height.
     * @param text The text to build from.
     * @param chars The set of unique characters in the text.
     */
    void build_tree(std::string &text, const std::set<uint8_t> &chars) {
        m_sigma = chars.size();
        m_dict.reserve(m_sigma);
        m_dict_inv.reserve(m_sigma);

        uint8_t ci = 0;
        for (const uint8_t &c: chars) {
            m_dict.push_back(c);
            m_dict_inv[c] = ci++;
        }

        // equivalent to taking floor(log2), compiler should be smart enough to replace with a single BSRL instruction
        uint8_t tmp = m_sigma;
        while (tmp >>= 1) {
            m_height++;
        }
        // account for any dangling leaves
        if (m_sigma > 1 << m_height) {
            m_height++;
        }

        m_bit_vector.set_size(m_size * m_height);
        build_bit_vector(text);
        m_bit_vector.build_rank();
    }

    /**
     * Build the bit vector representation of the wavelet tree.
     * @param initial_sequence The initial text sequence to partition.
     */
    void build_bit_vector(std::string &initial_sequence) {
        struct frame {
            std::string sequence;
            uint64_t offset;
            uint8_t height;
            uint8_t left;
            uint8_t right;
        };

        // equivalent dfs to recursion
        std::stack<frame> stack;
        stack.emplace(std::move(initial_sequence), 0, 0, 0, m_sigma);

        while (!stack.empty()) {
            auto [sequence, offset, height, left, right] = std::move(stack.top());
            stack.pop();

            const uint64_t size_half = (sequence.size() + 1) / 2;
            const uint8_t pivot = left + (right - left + 1) / 2;
            std::string left_side, right_side;

            left_side.reserve(size_half);
            right_side.reserve(size_half);

            for (uint64_t i = 0; i < sequence.size(); ++i) {
                if (m_dict_inv[sequence[i]] < pivot) {
                    left_side.push_back(sequence[i]);
                } else {
                    m_bit_vector.flip(m_size * height + offset + i);
                    right_side.push_back(sequence[i]);
                }
            }

            // dfs order
            if (right - pivot > 1) {
                stack.emplace(std::move(right_side), offset + left_side.size(), height + 1, pivot, right);
            }
            if (pivot - left > 1) {
                stack.emplace(std::move(left_side), offset, height + 1, left, pivot);
            }
        }
    }
};

inline std::ostream &operator<<(std::ostream &os, const wavelet_tree &wt) {
    const uint64_t wt_size = wt.size();
    const bit_vector &bv = wt.get_bit_vector();
    const uint64_t bv_size = bv.size();

    for (uint64_t i = 0; i < bv_size; i++) {
        os << static_cast<int>(bv.access(i));
        if (i < bv_size - 1 && (i + 1) % wt_size == 0) {
            os << '\n';
        }
    }

    return os;
}
