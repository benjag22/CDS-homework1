#include <filesystem>
#include <iostream>

#include "fm_index.hpp"

int main() {
    const auto base_path = std::filesystem::path(__builtin_FILE()).parent_path().parent_path() / "texts";
    const fm_index fmi(base_path / "example.txt");

    std::cout << fmi.count("on", WTType::BRUTE_FORCE) << std::endl;

    return 0;
}
