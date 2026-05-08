#include <filesystem>

#include "fm_index.hpp"
#include "util.hpp"

int main() {
    const auto base_path = std::filesystem::path(__builtin_FILE()).parent_path().parent_path() / "texts";
    const std::string contents = read_file(base_path / "example.txt");

    const fm_index fmi(base_path / "example.txt");

    return 0;
}
