#include "parameter.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "problem.h"

namespace {
std::string trim(const std::string &value) {
    std::size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) {
        ++start;
    }

    std::size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }

    return value.substr(start, end - start);
}

std::string to_lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::vector<std::string> collect_tokens(std::istringstream &stream) {
    std::vector<std::string> tokens;
    std::string token;
    while (stream >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

double parse_double(const std::string &token, std::size_t line_number, const std::string &field) {
    try {
        size_t idx = 0;
        const double value = std::stod(token, &idx);
        if (idx != token.size()) {
            throw std::invalid_argument("contains trailing characters");
        }
        return value;
    } catch (const std::exception &ex) {
        throw std::runtime_error("Failed to parse double for '" + field + "' on line " + std::to_string(line_number) +
                                 ": " + ex.what());
    }
}

std::size_t parse_size(const std::string &token, std::size_t line_number, const std::string &field) {
    try {
        size_t idx = 0;
        const unsigned long parsed = std::stoul(token, &idx);
        if (idx != token.size()) {
            throw std::invalid_argument("contains trailing characters");
        }
        return static_cast<std::size_t>(parsed);
    } catch (const std::exception &ex) {
        throw std::runtime_error("Failed to parse size for '" + field + "' on line " + std::to_string(line_number) +
                                 ": " + ex.what());
    }
}

void ensure_token_count(const std::vector<std::string> &tokens, std::size_t expected, const std::string &field,
                        std::size_t line_number) {
    if (expected != 0 && tokens.size() != expected) {
        throw std::runtime_error("Field '" + field + "' on line " + std::to_string(line_number) +
                                 " expected " + std::to_string(expected) + " entries but found " +
                                 std::to_string(tokens.size()));
    }
}

void synchronize_distribution_aliases(OptimizationParameters &params) {
    params.distribution_index_crossover = params.crossover_distribution_index;
    params.distribution_index_mutation = params.mutation_distribution_index;
}

void ensure_zdt4_defaults(OptimizationParameters &params,
                          std::size_t &declared_variables,
                          std::size_t &declared_objectives) {
    const std::string canonical = canonicalize_problem_name(params.problem_name);
    if (canonical != "ZDT4") {
        return;
    }

    constexpr std::size_t expected_variables = 10;
    constexpr std::size_t expected_objectives = 2;

    if (declared_variables != 0 && declared_variables != expected_variables) {
        std::cerr << "Warning: ZDT4 expects " << expected_variables
                  << " decision variables. Overriding declared count of " << declared_variables << " with "
                  << expected_variables << "\n";
    }
    declared_variables = expected_variables;

    if (params.variable_names.size() != expected_variables) {
        if (!params.variable_names.empty()) {
            std::cerr << "Warning: Overriding ZDT4 variable names to x1..x" << expected_variables << "\n";
        }
        params.variable_names.resize(expected_variables);
        for (std::size_t i = 0; i < expected_variables; ++i) {
            params.variable_names[i] = "x" + std::to_string(i + 1);
        }
    }

    std::vector<double> lower(expected_variables, -5.0);
    lower[0] = 0.0;
    std::vector<double> upper(expected_variables, 5.0);
    upper[0] = 1.0;

    if (params.variable_lower_bounds.size() != expected_variables) {
        if (!params.variable_lower_bounds.empty()) {
            std::cerr << "Warning: Overriding ZDT4 lower bounds to default range." << '\n';
        }
        params.variable_lower_bounds = lower;
    } else {
        bool mismatch = false;
        for (std::size_t i = 0; i < expected_variables; ++i) {
            const double expected = lower[i];
            if (std::abs(params.variable_lower_bounds[i] - expected) > 1e-9) {
                mismatch = true;
                break;
            }
        }
        if (mismatch) {
            std::cerr << "Warning: Adjusting ZDT4 lower bounds to [0,1] for x1 and [-5,5] for others." << '\n';
            params.variable_lower_bounds = lower;
        }
    }

    if (params.variable_upper_bounds.size() != expected_variables) {
        if (!params.variable_upper_bounds.empty()) {
            std::cerr << "Warning: Overriding ZDT4 upper bounds to default range." << '\n';
        }
        params.variable_upper_bounds = upper;
    } else {
        bool mismatch = false;
        for (std::size_t i = 0; i < expected_variables; ++i) {
            const double expected = upper[i];
            if (std::abs(params.variable_upper_bounds[i] - expected) > 1e-9) {
                mismatch = true;
                break;
            }
        }
        if (mismatch) {
            std::cerr << "Warning: Adjusting ZDT4 upper bounds to [0,1] for x1 and [-5,5] for others." << '\n';
            params.variable_upper_bounds = upper;
        }
    }

    if (declared_objectives != 0 && declared_objectives != expected_objectives) {
        std::cerr << "Warning: ZDT4 uses " << expected_objectives
                  << " objectives. Overriding declared count of " << declared_objectives << "\n";
    }
    declared_objectives = expected_objectives;

    if (params.objective_names.size() != expected_objectives) {
        if (!params.objective_names.empty()) {
            std::cerr << "Warning: Overriding ZDT4 objective names to f1 and f2." << '\n';
        }
        params.objective_names = {"f1", "f2"};
    }
}

} // namespace

OptimizationParameters load_parameters_from_file(const std::string &path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Unable to open parameter file: " + path);
    }

    OptimizationParameters params{};
    std::size_t declared_variables = 0;
    std::size_t declared_objectives = 0;

    std::string line;
    std::size_t line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        const auto comment_pos = line.find('#');
        if (comment_pos != std::string::npos) {
            line = line.substr(0, comment_pos);
        }

        line = trim(line);
        if (line.empty()) {
            continue;
        }

        std::istringstream stream(line);
        std::string key;
        stream >> key;
        if (key.empty()) {
            continue;
        }

        const std::string normalized_key = to_lower(key);
        std::vector<std::string> tokens = collect_tokens(stream);

        if (normalized_key == "problem" || normalized_key == "problem_name") {
            if (tokens.empty()) {
                throw std::runtime_error("Problem name missing on line " + std::to_string(line_number));
            }
            params.problem_name = tokens.front();
        } else if (normalized_key == "num_variables" || normalized_key == "number_of_variables") {
            if (tokens.size() != 1) {
                throw std::runtime_error("num_variables expects exactly one value on line " + std::to_string(line_number));
            }
            declared_variables = parse_size(tokens.front(), line_number, key);
        } else if (normalized_key == "variable_names") {
            ensure_token_count(tokens, declared_variables, key, line_number);
            params.variable_names = tokens;
            if (declared_variables == 0) {
                declared_variables = params.variable_names.size();
            }
        } else if (normalized_key == "variable_lower_bounds" || normalized_key == "lower_bounds") {
            ensure_token_count(tokens, declared_variables, key, line_number);
            params.variable_lower_bounds.clear();
            params.variable_lower_bounds.reserve(tokens.size());
            for (const auto &token : tokens) {
                params.variable_lower_bounds.push_back(parse_double(token, line_number, key));
            }
            if (declared_variables == 0) {
                declared_variables = params.variable_lower_bounds.size();
            }
        } else if (normalized_key == "variable_upper_bounds" || normalized_key == "upper_bounds") {
            ensure_token_count(tokens, declared_variables, key, line_number);
            params.variable_upper_bounds.clear();
            params.variable_upper_bounds.reserve(tokens.size());
            for (const auto &token : tokens) {
                params.variable_upper_bounds.push_back(parse_double(token, line_number, key));
            }
            if (declared_variables == 0) {
                declared_variables = params.variable_upper_bounds.size();
            }
        } else if (normalized_key == "num_objectives" || normalized_key == "number_of_objectives") {
            if (tokens.size() != 1) {
                throw std::runtime_error("num_objectives expects exactly one value on line " + std::to_string(line_number));
            }
            declared_objectives = parse_size(tokens.front(), line_number, key);
        } else if (normalized_key == "objective_names") {
            ensure_token_count(tokens, declared_objectives, key, line_number);
            params.objective_names = tokens;
            if (declared_objectives == 0) {
                declared_objectives = params.objective_names.size();
            }
        } else if (normalized_key == "parent_population_size" || normalized_key == "population_size") {
            if (tokens.size() != 1) {
                throw std::runtime_error("population_size expects exactly one value on line " + std::to_string(line_number));
            }
            params.population_size = parse_size(tokens.front(), line_number, key);
        } else if (normalized_key == "offspring_population_size") {
            if (tokens.size() != 1) {
                throw std::runtime_error(
                    "offspring_population_size expects exactly one value on line " + std::to_string(line_number));
            }
            params.offspring_population_size = parse_size(tokens.front(), line_number, key);
        } else if (normalized_key == "max_generation" || normalized_key == "max_generations") {
            if (tokens.size() != 1) {
                throw std::runtime_error("max_generation expects exactly one value on line " + std::to_string(line_number));
            }
            params.max_generations = parse_size(tokens.front(), line_number, key);
        } else if (normalized_key == "crossover_probability") {
            if (tokens.size() != 1) {
                throw std::runtime_error(
                    "crossover_probability expects exactly one value on line " + std::to_string(line_number));
            }
            params.crossover_probability = parse_double(tokens.front(), line_number, key);
        } else if (normalized_key == "mutation_probability" || normalized_key == "mutation_porbability") {
            if (tokens.size() != 1) {
                throw std::runtime_error(
                    "mutation_probability expects exactly one value on line " + std::to_string(line_number));
            }
            params.mutation_probability = parse_double(tokens.front(), line_number, key);
        } else if (normalized_key == "crossover_distribution_index") {
            if (tokens.size() != 1) {
                throw std::runtime_error("crossover_distribution_index expects one value on line " +
                                         std::to_string(line_number));
            }
            params.crossover_distribution_index = parse_double(tokens.front(), line_number, key);
        } else if (normalized_key == "mutation_distribution_index") {
            if (tokens.size() != 1) {
                throw std::runtime_error("mutation_distribution_index expects one value on line " +
                                         std::to_string(line_number));
            }
            params.mutation_distribution_index = parse_double(tokens.front(), line_number, key);
        } else if (normalized_key == "random_seed") {
            if (tokens.size() != 1) {
                throw std::runtime_error("random_seed expects exactly one value on line " + std::to_string(line_number));
            }
            params.random_seed = static_cast<unsigned int>(parse_size(tokens.front(), line_number, key));
        } else {
            std::cerr << "Warning: Unrecognized parameter key '" << key << "' on line " << line_number << "\n";
        }
    }

    synchronize_distribution_aliases(params);
    ensure_zdt4_defaults(params, declared_variables, declared_objectives);

    if (!params.variable_names.empty() && declared_variables != params.variable_names.size()) {
        throw std::runtime_error("Variable name count does not match the declared number of variables");
    }
    if (!params.variable_lower_bounds.empty() && declared_variables != params.variable_lower_bounds.size()) {
        throw std::runtime_error("Lower bound count does not match the declared number of variables");
    }
    if (!params.variable_upper_bounds.empty() && declared_variables != params.variable_upper_bounds.size()) {
        throw std::runtime_error("Upper bound count does not match the declared number of variables");
    }
    if (!params.objective_names.empty() && declared_objectives != params.objective_names.size()) {
        throw std::runtime_error("Objective name count does not match the declared number of objectives");
    }

    if (params.problem_name.empty()) {
        throw std::runtime_error("Problem name must be specified in the parameter file");
    }

    if (params.population_size == 0) {
        throw std::runtime_error("Population size must be greater than zero");
    }

    if (params.variable_names.empty() && declared_variables != 0) {
        params.variable_names.resize(declared_variables);
        for (std::size_t i = 0; i < declared_variables; ++i) {
            params.variable_names[i] = "x" + std::to_string(i + 1);
        }
    }

    if (!params.variable_lower_bounds.empty() && !params.variable_upper_bounds.empty() &&
        params.variable_lower_bounds.size() == params.variable_upper_bounds.size()) {
        for (std::size_t i = 0; i < params.variable_lower_bounds.size(); ++i) {
            if (params.variable_lower_bounds[i] > params.variable_upper_bounds[i]) {
                throw std::runtime_error("Lower bound exceeds upper bound for variable index " + std::to_string(i));
            }
        }
    }

    if (params.offspring_population_size != params.population_size) {
        std::cerr << "Warning: Offspring population size (" << params.offspring_population_size
                  << ") differs from parent population size (" << params.population_size << ")\n";
    }

    if (!is_problem_recorded(params.problem_name)) {
        std::cerr << "Warning: Problem '" << params.problem_name
                  << "' is not recorded in problem.cpp and may not be evaluated by this demo.\n";
    }

    return params;
}

OptimizationParameters load_parameters_from_cli(int argc, char **argv) {
    std::string parameter_path;
    if (argc > 1 && argv[1] != nullptr) {
        parameter_path = argv[1];
    } else {
        parameter_path = "input.txt";
        std::cerr << "Warning: No parameter file specified. Using default path '" << parameter_path << "'.\n";
    }

    OptimizationParameters params = load_parameters_from_file(parameter_path);
    return params;
}

