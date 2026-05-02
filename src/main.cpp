#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>
#include "wavelet_tree.hpp"

int main() {
    std::string a =  "abbbsbbabannabbabb";
    std::vector<uint32_t> seql;

    for (const auto e: a) {
        seql.push_back(e - 'a');
    }

    std::vector<uint32_t> seq = {5,7,13,21,4,5,50,23,48,5,19};
    WaveletTree wt(seq);
    for (uint32_t i= 0; i < seq.size(); ++i) {
        uint32_t e0 = wt.access(i);
        uint32_t e1 = seq[i];
        std::cout << e0 << " " << e1 << "\n";
        assert(e0 == e1);
    }
    std::cout <<wt.rank(5,0)<< std::endl;
    assert(wt.rank(5,0) == 0);
    assert(wt.rank(5,1) == 1);
    assert(wt.rank(5,5) == 1);
    assert(wt.rank(5,6) == 2);
    assert(wt.rank(5,9) == 2);

}
