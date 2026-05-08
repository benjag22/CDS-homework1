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
    const std::string label;
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
        .label = "runs",
        .value = 32,
        .min = 32,
    };
    Arg lower{
        .name = "-l",
        .label = "lower",
        .value = 1,
        .min = 1,
    };
    Arg upper{
        .name = "-u",
        .label = "upper",
        .value = 1024,
        .min = 1,
        .validator = [this]() -> ArgValidationResult {
            return {upper > lower, upper.label + " must be > " + lower.label};
        },
    };
    Arg step{
        .name = "-s",
        .label = "step",
        .value = 2,
        .min = 1,
    };

private:
    typedef std::vector<Arg *> args_list_t;
    args_list_t m_args_list{
        &runs,
        &lower,
        &upper,
        &step,
    };
    const std::unordered_map<std::string, Arg *> m_args_map{
        {runs.name, &runs},
        {lower.name, &lower},
        {upper.name, &upper},
        {step.name, &step},
    };

public:
    [[nodiscard]] Arg *get(const std::string &name) const {
        return m_args_map.contains(name) ? m_args_map.at(name) : nullptr;
    }

    [[nodiscard]] std::string usage_string(const std::string &cmd) const {
        std::stringstream ss;

        ss << "Usage: " + cmd + " <text_file_path> [options]\n\n"
            << "Options:\n\n";

        for (const auto &arg: m_args_list) {
            ss << "  " << arg->name << " : " << arg->label << "\n"
                << "\tdefault: " << arg->value << "\n";

            if (arg->min != std::numeric_limits<uint64_t>::min()) {
                ss << "\tmin: " << arg->min << "\n";
            }

            if (arg->max != std::numeric_limits<uint64_t>::max()) {
                ss << "\tmax: " << arg->max << "\n";
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

    explicit ParsedArgs(const char *text_file_path, const Args &args)
        : text_file_path(text_file_path),
          runs(args.runs.value),
          lower(args.lower.value),
          upper(args.upper.value),
          step(args.step.value) {
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

    for (int i = 2; i < argc; i += 2) {
        Arg *arg_ptr = args.get(argv[i]);

        if (arg_ptr == nullptr) {
            cerr << argv[0] << ": " << "invalid option " << argv[i] << "\n\n"
                << args.usage_string(argv[0]) << endl;
            exit(EXIT_FAILURE);
        }

        if (i + 1 >= argc) {
            break;
        }

        auto &[name, label, value, min, max, validator] = *arg_ptr;

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
