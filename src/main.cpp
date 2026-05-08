#include <filesystem>

#include "args.hpp"
#include "uhr/uhr.hpp"

int main(const int argc, const char *const argv[]) {
    const ParsedArgs &args = parse_args(argc, argv);

    const auto input_file_path = std::filesystem::path(args.text_file_path);
    const auto results_file_path = std::filesystem::path(__builtin_FILE()).parent_path().parent_path()
        / "data"
        / input_file_path.filename().replace_extension(".csv");

    uhr(input_file_path, results_file_path, args.runs, args.lower, args.upper, args.step, true);

    return 0;
}
