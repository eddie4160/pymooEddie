#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "initpop.h"
#include "parameter.h"
#include "problem.h"
#include "sort.h"

OptimizationParameters load_default_parameters() {
    OptimizationParameters params{};

    params.variable_names = {"x1", "x2", "x3", "x4", "x5"};
    params.objective_names = {"Objective 1", "Objective 2"};

    params.variable_lower_bounds = {0.0, 0.0, 0.0, 0.0, 0.0};
    params.variable_upper_bounds = {1.0, 1.0, 1.0, 1.0, 1.0};

    return params;
}

void print_parameters(const OptimizationParameters &params) {
    std::cout << "NSGA-II configuration" << '\n';
    std::cout << "----------------------" << '\n';
    std::cout << "Population size: " << params.population_size << '\n';
    std::cout << "Offspring population size: " << params.offspring_population_size << '\n';
    std::cout << "Max generations: " << params.max_generations << '\n';
    std::cout << "Crossover probability: " << params.crossover_probability << '\n';
    std::cout << "Mutation probability: " << params.mutation_probability << '\n';
    std::cout << "SBX distribution index: " << params.distribution_index_crossover << '\n';
    std::cout << "Polynomial mutation index: " << params.distribution_index_mutation << '\n';
    std::cout << "Random seed: " << params.random_seed << '\n';

    std::cout << "Design variables:" << '\n';
    for (std::size_t i = 0; i < params.variable_names.size(); ++i) {
        const auto &name = params.variable_names[i];
        const auto lower = i < params.variable_lower_bounds.size() ? params.variable_lower_bounds[i] : 0.0;
        const auto upper = i < params.variable_upper_bounds.size() ? params.variable_upper_bounds[i] : 0.0;
        std::cout << "  - " << name << " in [" << lower << ", " << upper << "]" << '\n';
    }

    std::cout << "Objectives:" << '\n';
    for (const auto &name : params.objective_names) {
        std::cout << "  - " << name << '\n';
    }
}

void print_population_sample(const Population &population, std::size_t count = 5) {
    std::cout << "\nLatin Hypercube Initial Population (first "
              << std::min(count, population.size()) << " individuals)" << '\n';
    std::cout << "-----------------------------------------------------" << '\n';
    const std::size_t display_count = std::min(count, population.size());
    for (std::size_t i = 0; i < display_count; ++i) {
        std::cout << "Individual " << (i + 1) << ": ";
        for (std::size_t j = 0; j < population[i].size(); ++j) {
            std::cout << std::fixed << std::setprecision(4) << population[i][j];
            if (j + 1 < population[i].size()) {
                std::cout << ", ";
            }
        }
        std::cout << '\n';
    }
}

std::string sanitize_identifier(const std::string &name, const std::string &fallback_prefix, std::size_t index) {
    std::string sanitized;
    sanitized.reserve(name.size());
    for (unsigned char ch : name) {
        if (std::isalnum(ch) || ch == '_') {
            sanitized.push_back(static_cast<char>(ch));
        } else if (ch == ' ') {
            sanitized.push_back('_');
        }
    }

    if (sanitized.empty()) {
        sanitized = fallback_prefix + std::to_string(index + 1);
    }

    return sanitized;
}

void write_population_report(const std::string &path,
                             const OptimizationParameters &params,
                             const Population &population,
                             const std::vector<std::vector<double>> &objectives,
                             const NonDominatedSortResult &sort_result) {
    if (population.size() != objectives.size() || population.size() != sort_result.ranks.size()) {
        throw std::runtime_error("Population, objective, and rank counts must match to write report");
    }

    std::ofstream out(path);
    if (!out) {
        throw std::runtime_error("Failed to open population report file: " + path);
    }

    out << "# NSGA-II initial population export" << '\n';
    out << "# index front";

    for (std::size_t i = 0; !objectives.empty() && i < objectives.front().size(); ++i) {
        const std::string label = i < params.objective_names.size()
                                       ? sanitize_identifier(params.objective_names[i], "objective", i)
                                       : "objective" + std::to_string(i + 1);
        out << ' ' << label;
    }

    for (std::size_t i = 0; i < params.variable_names.size(); ++i) {
        const std::string label = sanitize_identifier(params.variable_names[i], "x", i);
        out << ' ' << label;
    }

    out << '\n';

    for (std::size_t i = 0; i < population.size(); ++i) {
        const std::size_t front = sort_result.ranks[i] + 1; // convert to 1-based indexing
        out << i << ' ' << front;

        if (i < objectives.size()) {
            for (const double value : objectives[i]) {
                out << ' ' << std::setprecision(12) << value;
            }
        }

        if (i < population.size()) {
            for (const double variable : population[i]) {
                out << ' ' << std::setprecision(12) << variable;
            }
        }

        out << '\n';
    }
}

int main(int argc, char **argv) {
    try {
        const auto params = load_default_parameters();
        print_parameters(params);

        const auto population = latin_hypercube_population(params);
        print_population_sample(population);

        if (!population.empty()) {
            const auto objectives = evaluate_zdt4(population.front());
            std::cout << "\nZDT4 objectives for first individual: "
                      << std::fixed << std::setprecision(6) << objectives[0] << ", "
                      << objectives[1] << '\n';

            const auto objective_matrix = evaluate_zdt4_population(population);
            const auto sort_result = fast_non_dominated_sort(objective_matrix);

            std::cout << "\nNon-dominated sorting summary:" << '\n';
            std::cout << "  Total fronts: " << sort_result.fronts.size() << '\n';
            if (!sort_result.fronts.empty()) {
                std::cout << "  First front indices: ";
                for (std::size_t i = 0; i < sort_result.fronts.front().size(); ++i) {
                    std::cout << sort_result.fronts.front()[i];
                    if (i + 1 < sort_result.fronts.front().size()) {
                        std::cout << ", ";
                    }
                }
                std::cout << '\n';
            }

            const std::string output_path = argc > 1 ? argv[1] : "Eddie/initial_population.txt";
            write_population_report(output_path, params, population, objective_matrix, sort_result);
            std::cout << "\nWrote initial population report to " << output_path << '\n';
        }
    } catch (const std::exception &ex) {
        std::cerr << "Failed to initialize NSGA-II parameters: " << ex.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
