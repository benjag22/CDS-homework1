#include <cassert>
#include <filesystem>
#include <vector>

#include "bit_vector.hpp"
#include "wavelet_tree.hpp"

int main() {
    constexpr uint64_t value = 0b0111100100110000000101011100110001111001001100000001010111001100ull;
    const std::vector<uint8_t> bits{0,0,1,1,0,0,1,1,1,0,1,0,1,0,0,0,0,0,0,0,1,1,0,0,1,0,0,1,1,1,1,0,0,0,1,1,0,0,1,1,1,0,1,0,1,0,0,0,0,0,0,0,1,1,0,0,1,0,0,1,1,1,1,0,0,0,1,1,0,0,1,1,1,0,1,0,1,0,0,0,0,0,0,0,1,1,0,0,1,0,0,1,1,1,1,0,0,0,1,1,0,0,1,1,1,0,1,0,1,0,0,0,0,0,0,0,1,1,0,0,1,0,0,1,1,1,1,0,0,0,1,1,0,0,1,1,1,0,1,0,1,0,0,0,0,0,0,0,1,1,0,0,1,0,0,1,1,1,1,0,0,0,1,1,0,0,1,1,1,0,1,0,1,0,0,0,0,0,0,0,1,1,0,0,1,0,0,1,1,1,1,0,0,0,1,1,0,0,1,1,1,0,1,0,1,0,0,0,0,0,0,0,1,1,0,0,1,0,0,1,1,1,1,0,0,0,1,1,0,0,1,1,1,0,1,0,1,0,0,0,0,0,0,0,1,1,0,0,1,0,0,1,1,1,1,0};
    const std::vector<uint64_t> ranks{0,0,1,2,2,2,3,4,5,5,6,6,7,7,7,7,7,7,7,7,8,9,9,9,10,10,10,11,12,13,14,14,14,14,15,16,16,16,17,18,19,19,20,20,21,21,21,21,21,21,21,21,22,23,23,23,24,24,24,25,26,27,28,28,28,28,29,30,30,30,31,32,33,33,34,34,35,35,35,35,35,35,35,35,36,37,37,37,38,38,38,39,40,41,42,42,42,42,43,44,44,44,45,46,47,47,48,48,49,49,49,49,49,49,49,49,50,51,51,51,52,52,52,53,54,55,56,56,56,56,57,58,58,58,59,60,61,61,62,62,63,63,63,63,63,63,63,63,64,65,65,65,66,66,66,67,68,69,70,70,70,70,71,72,72,72,73,74,75,75,76,76,77,77,77,77,77,77,77,77,78,79,79,79,80,80,80,81,82,83,84,84,84,84,85,86,86,86,87,88,89,89,90,90,91,91,91,91,91,91,91,91,92,93,93,93,94,94,94,95,96,97,98,98,98,98,99,100,100,100,101,102,103,103,104,104,105,105,105,105,105,105,105,105,106,107,107,107,108,108,108,109,
        110,
        111,
        112,
        112
    };
    bit_vector bv(std::vector(64, value));

    assert(bits.size() == ranks.size());
    assert(bits.size() <= bv.size());

    for (uint64_t i = 0; i < bits.size(); i++) {
        const uint64_t bit = bv.access(i);
        const uint64_t rank = bv.rank_1(i);
        // std::cout << i << "\n";
        assert(bit == bits[i % bits.size()]);
        assert(rank == ranks[i]);
    }

    const wavelet_tree wt(std::filesystem::path(__builtin_FILE()).parent_path().parent_path() / "texts/example.txt");

    const auto wt_string =
        "1100011110001110000000011100110\n"
        "1100001000010011000110010011101\n"
        "0010010100001110111110011010111\n"
        "0000010011001001001101000110010\n"
        "0000001000000000000000000000000\n";

    assert(wt.to_string() == wt_string);

    assert(wt.access(0) == 'o');
    assert(wt.access(1) == 'n');
    assert(wt.access(2) == 'c');
    assert(wt.access(3) == 'e');
    assert(wt.access(4) == ' ');
    assert(wt.access(5) == 'u');
    assert(wt.access(6) == 'p');
    assert(wt.access(7) == 'o');
    assert(wt.access(8) == 'n');
    assert(wt.access(9) == ' ');
    assert(wt.access(10) == 'a');
    assert(wt.access(11) == ' ');
    assert(wt.access(12) == 't');
    assert(wt.access(13) == 'i');
    assert(wt.access(14) == 'm');
    assert(wt.access(15) == 'e');
    assert(wt.access(16) == ' ');
    assert(wt.access(17) == 'a');
    assert(wt.access(18) == ' ');
    assert(wt.access(19) == 'P');
    assert(wt.access(20) == 'h');
    assert(wt.access(21) == 'D');
    assert(wt.access(22) == ' ');
    assert(wt.access(23) == 's');
    assert(wt.access(24) == 't');
    assert(wt.access(25) == 'u');
    assert(wt.access(26) == 'd');
    assert(wt.access(27) == 'e');
    assert(wt.access(28) == 'n');
    assert(wt.access(29) == 't');
    assert(wt.access(30) == '.');

    assert(wt.rank(0, 'o') == 1);
    assert(wt.rank(6, 'o') == 1);
    assert(wt.rank(7, 'o') == 2);
    assert(wt.rank(30, 'o') == 2);

    assert(wt.rank(0, 'n') == 0);
    assert(wt.rank(1, 'n') == 1);
    assert(wt.rank(7, 'n') == 1);
    assert(wt.rank(8, 'n') == 2);
    assert(wt.rank(27, 'n') == 2);
    assert(wt.rank(28, 'n') == 3);
    assert(wt.rank(30, 'n') == 3);

    assert(wt.rank(1, 'c') == 0);
    assert(wt.rank(2, 'c') == 1);
    assert(wt.rank(30, 'c') == 1);

    assert(wt.rank(2, 'e') == 0);
    assert(wt.rank(3, 'e') == 1);
    assert(wt.rank(14, 'e') == 1);
    assert(wt.rank(15, 'e') == 2);
    assert(wt.rank(26, 'e') == 2);
    assert(wt.rank(27, 'e') == 3);
    assert(wt.rank(30, 'e') == 3);

    assert(wt.rank(3, ' ') == 0);
    assert(wt.rank(4, ' ') == 1);
    assert(wt.rank(8, ' ') == 1);
    assert(wt.rank(9, ' ') == 2);
    assert(wt.rank(10, ' ') == 2);
    assert(wt.rank(11, ' ') == 3);
    assert(wt.rank(15, ' ') == 3);
    assert(wt.rank(16, ' ') == 4);
    assert(wt.rank(17, ' ') == 4);
    assert(wt.rank(18, ' ') == 5);
    assert(wt.rank(21, ' ') == 5);
    assert(wt.rank(22, ' ') == 6);
    assert(wt.rank(30, ' ') == 6);

    assert(wt.rank(4, 'u') == 0);
    assert(wt.rank(5, 'u') == 1);
    assert(wt.rank(24, 'u') == 1);
    assert(wt.rank(25, 'u') == 2);
    assert(wt.rank(30, 'u') == 2);

    assert(wt.rank(5, 'p') == 0);
    assert(wt.rank(6, 'p') == 1);
    assert(wt.rank(30, 'p') == 1);

    assert(wt.rank(9, 'a') == 0);
    assert(wt.rank(10, 'a') == 1);
    assert(wt.rank(16, 'a') == 1);
    assert(wt.rank(17, 'a') == 2);
    assert(wt.rank(30, 'a') == 2);

    assert(wt.rank(11, 't') == 0);
    assert(wt.rank(12, 't') == 1);
    assert(wt.rank(23, 't') == 1);
    assert(wt.rank(24, 't') == 2);
    assert(wt.rank(28, 't') == 2);
    assert(wt.rank(29, 't') == 3);
    assert(wt.rank(30, 't') == 3);

    assert(wt.rank(12, 'i') == 0);
    assert(wt.rank(13, 'i') == 1);
    assert(wt.rank(30, 'i') == 1);

    assert(wt.rank(13, 'm') == 0);
    assert(wt.rank(14, 'm') == 1);
    assert(wt.rank(30, 'm') == 1);

    assert(wt.rank(18, 'P') == 0);
    assert(wt.rank(19, 'P') == 1);
    assert(wt.rank(30, 'P') == 1);

    assert(wt.rank(19, 'h') == 0);
    assert(wt.rank(20, 'h') == 1);
    assert(wt.rank(30, 'h') == 1);

    assert(wt.rank(20, 'D') == 0);
    assert(wt.rank(21, 'D') == 1);
    assert(wt.rank(30, 'D') == 1);

    assert(wt.rank(25, 'd') == 0);
    assert(wt.rank(26, 'd') == 1);
    assert(wt.rank(30, 'd') == 1);

    assert(wt.rank(29, '.') == 0);
    assert(wt.rank(30, '.') == 1);

    return 0;
}
