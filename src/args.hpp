#pragma once

#include <cerrno>
#include <cstdint>
#include <functional>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

struct ArgValidationResult {
    const bool is_valid = true;
    const std::string message;
};

struct Arg {
    const std::string name;
    const std::string description;
    const bool flag = false;
    uint64_t value = 0;
    const uint64_t min = std::numeric_limits<uint64_t>::min();
    const uint64_t max = std::numeric_limits<uint64_t>::max();

    const std::function<ArgValidationResult()> validator = []() -> ArgValidationResult {
        return {};
    };

    auto operator<=>(const Arg &other) const {
        return value <=> other.value;
    }
};

struct Args {
    Arg runs{
        .name = "-r",
        .description = "Number of runs per test case.",
        .value = 32,
        .min = 32,
    };
    Arg lower{
        .name = "-l",
        .description = "Lower bound for test cases range.",
        .value = 1,
        .min = 1,
    };
    Arg upper{
        .name = "-u",
        .description = "Upper bound for test cases range.",
        .value = 1025,
        .min = 1,
        .validator = [this]() -> ArgValidationResult {
            return {upper > lower, upper.description + " must be > " + lower.description};
        },
    };
    Arg step{
        .name = "-s",
        .description = "Step of test cases.",
        .value = 2,
        .min = 1,
    };
    Arg mult_step{
        .name = "-m",
        .description = "Whether to use multiplicative steps instead of additive.",
        .flag = true,
        .value = false,
    };

private:
    typedef std::vector<Arg *> args_list_t;
    args_list_t m_args_list{
        &runs,
        &lower,
        &upper,
        &step,
        &mult_step,
    };
    const std::unordered_map<std::string, Arg *> m_args_map{
        {runs.name, &runs},
        {lower.name, &lower},
        {upper.name, &upper},
        {step.name, &step},
        {mult_step.name, &mult_step},
    };

public:
    [[nodiscard]] Arg *get(const std::string &name) const {
        return m_args_map.contains(name) ? m_args_map.at(name) : nullptr;
    }

    [[nodiscard]] std::string usage_string(const std::string &cmd) const {
        std::stringstream ss;

        ss << std::boolalpha;

        ss << "Usage: " + cmd + " <text_file_path> [options]\n\n"
            << "Options:\n\n";

        for (const auto &arg: m_args_list) {
            ss << "  " << arg->name << " : " << arg->description << "\n"
                << "       default: ";

            if (arg->flag) {
                ss << static_cast<bool>(arg->value);
            } else {
                ss << arg->value;
            }

            ss << "\n";

            if (arg->min != std::numeric_limits<uint64_t>::min()) {
                ss << "       min: " << arg->min << "\n";
            }

            if (arg->max != std::numeric_limits<uint64_t>::max()) {
                ss << "       max: " << arg->max << "\n";
            }

            ss << "\n";
        }

        return ss.str();
    }

    args_list_t::iterator begin() {
        return m_args_list.begin();
    }

    args_list_t::iterator end() {
        return m_args_list.begin();
    }
};

struct ParsedArgs {
    const char *text_file_path;
    const uint64_t runs;
    const uint64_t lower;
    const uint64_t upper;
    const uint64_t step;
    const bool mult_step;

    explicit ParsedArgs(const char *text_file_path, const Args &args)
        : text_file_path(text_file_path),
          runs(args.runs.value),
          lower(args.lower.value),
          upper(args.upper.value),
          step(args.step.value),
          mult_step(args.mult_step.value) {
    }
};

inline ParsedArgs parse_args(const int argc, const char *const *const argv) {
    using std::cerr, std::endl, std::exit;

    Args args;

    if (argc < 2) {
        cerr << args.usage_string(argv[0]) << endl;
        exit(EXIT_FAILURE);
    }

    const char *text_file_path = argv[1];

    int i = 2;
    while (i < argc) {
        Arg *arg_ptr = args.get(argv[i]);

        if (arg_ptr == nullptr) {
            cerr << argv[0] << ": " << "invalid option " << argv[i] << "\n\n"
                << args.usage_string(argv[0]) << endl;
            exit(EXIT_FAILURE);
        }

        auto &[name, label, flag, value, min, max, validator] = *arg_ptr;

        if (flag) {
            value = true;
            i++;
        }

        if (i + 1 >= argc) {
            break;
        }

        errno = 0;
        char *end;
        const char *string = argv[i + 1];
        const uint64_t parsed_value = std::strtoull(string, &end, 10);

        if (string == end || errno == ERANGE) {
            cerr << argv[0] << ": " << "invalid option value " << argv[i + 1] << "\n\n"
                << args.usage_string(argv[0]) << endl;
            exit(EXIT_FAILURE);
        }

        value = parsed_value;

        if (value < min) {
            cerr << argv[0] << ": " << label << " must be >= " << min << "\n\n"
                << args.usage_string(argv[0]) << endl;
            exit(EXIT_FAILURE);
        }

        if (value > max) {
            cerr << argv[0] << ": " << label << " must be <= " << max << "\n\n"
                << args.usage_string(argv[0]) << endl;
            exit(EXIT_FAILURE);
        }

        i += 2;
    }

    for (const auto *arg: args) {
        const auto &[is_valid, message] = arg->validator();

        if (!is_valid) {
            cerr << argv[0] << ": " << message << "\n\n"
                << args.usage_string(argv[0]) << endl;
            exit(EXIT_FAILURE);
        }
    }

    return ParsedArgs(text_file_path, args);
}
