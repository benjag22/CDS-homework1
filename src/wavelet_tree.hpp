#pragma once
#include <cmath>
#include <algorithm>
#include "bit_vector.hpp"

struct Node {
    bitVector representation{};
    Node* childs[2]{};

    explicit Node(const uint32_t len) {
        representation.resize(len);
        childs[0] = nullptr;
        childs[1] = nullptr;
    }

    ~Node() {
        delete childs[0];
        delete childs[1];
    }
};

class WaveletTree {
    uint32_t sigma;
    uint32_t n_size;
    int height;
    Node* root;

public:
    explicit WaveletTree(std::vector<uint32_t> const &sequence) {
        if (sequence.empty()) {
            root = nullptr;
            return;
        }
        sigma = 0;
        for (auto const &e : sequence) sigma = std::max(sigma, e);

        n_size = sequence.size();
        height = (sigma == 0) ? 0 : static_cast<int>(std::floor(std::log2(sigma)));
        root = buildTree(sequence, height);
    }

    ~WaveletTree() { delete root; }

    [[nodiscard]] uint32_t access(uint32_t index) const {
        int h = height;
        uint32_t element = 0;
        const Node* aux = root;
        while (aux != nullptr) {
            if (aux->representation.access(index) == 0) {
                index = aux->representation.rank_0(static_cast<int>(index)) - 1;
                aux = aux->childs[0];
            } else {
                element |= (1 << h);
                index = aux->representation.rank_1(static_cast<int>(index)) - 1;
                aux = aux->childs[1];
            }
            h--;
        }
        return element;
    }

    [[nodiscard]] uint32_t rank(const uint32_t symbol, uint32_t index) const {
        if (index == 0) return 0;
        index -= 1;
        int h = height;
        const Node* aux = root;
        while (aux != nullptr) {
            if ((symbol >> h) & 1) {
                const uint32_t cnt = aux->representation.rank_1(static_cast<int>(index));
                if (cnt == 0) return 0;
                index = cnt - 1;
                aux = aux->childs[1];
            } else {
                const uint32_t cnt = aux->representation.rank_0(static_cast<int>(index));
                if (cnt == 0) return 0;
                index = cnt - 1;
                aux = aux->childs[0];
            }
            h--;
        }
        return index + 1;
    }

private:
    static Node* buildTree(const std::vector<uint32_t>& sequence, const int h) {
        if (sequence.empty() || h < -1) return nullptr;

        const auto node = new Node(sequence.size());
        std::vector<uint32_t> left_side, right_side;

        for (int i = 0; i < sequence.size(); ++i) {
            if ((sequence[i] >> h) & 1) {
                node->representation.set(i);
                right_side.push_back(sequence[i]);
            } else {
                left_side.push_back(sequence[i]);
            }
        }

        node->representation.build_rank();

        if (h >= 0) {
            node->childs[0] = buildTree(left_side, h - 1);
            node->childs[1] = buildTree(right_side, h - 1);
        }
        return node;
    }
};
