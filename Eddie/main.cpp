#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "initpop.h"
#include "parameter.h"
#include "problem.h"
#include "sort.h"

void print_parameters(const OptimizationParameters &params) {
    std::cout << "NSGA-II configuration" << '\n';
    std::cout << "----------------------" << '\n';
    std::cout << "Problem: " << params.problem_name << '\n';
    std::cout << "Population size: " << params.population_size << '\n';
    std::cout << "Offspring population size: " << params.offspring_population_size << '\n';
    std::cout << "Max generations: " << params.max_generations << '\n';
    std::cout << "Crossover probability: " << params.crossover_probability << '\n';
    std::cout << "Mutation probability: " << params.mutation_probability << '\n';
    std::cout << "SBX distribution index: " << params.crossover_distribution_index << '\n';
    std::cout << "Polynomial mutation index: " << params.mutation_distribution_index << '\n';
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
        const OptimizationParameters params = load_parameters_from_cli(argc, argv);
        print_parameters(params);

        const Population population = latin_hypercube_population(params);
        print_population_sample(population);

        std::vector<std::vector<double>> objective_matrix;
        bool evaluation_available = false;

        try {
            objective_matrix = evaluate_problem_population(params.problem_name, population);
            evaluation_available = true;
        } catch (const std::exception &ex) {
            std::cerr << "Warning: Failed to evaluate problem '" << params.problem_name
                      << "': " << ex.what() << '\n';
        }

        if (evaluation_available && !objective_matrix.empty()) {
            const auto &first_objectives = objective_matrix.front();
            if (first_objectives.size() >= 2) {
                std::cout << "\nObjectives for first individual: " << std::fixed << std::setprecision(6)
                          << first_objectives[0];
                for (std::size_t i = 1; i < first_objectives.size(); ++i) {
                    std::cout << ", " << first_objectives[i];
                }
                std::cout << '\n';
            }

            const NonDominatedSortResult sort_result = fast_non_dominated_sort(objective_matrix);

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

            const std::string output_path = (argc > 2 && argv[2] != nullptr) ? argv[2] : "Eddie/initial_population.txt";
            write_population_report(output_path, params, population, objective_matrix, sort_result);
            std::cout << "\nWrote initial population report to " << output_path << '\n';
        } else {
            std::cout << "\nSkipping non-dominated sorting and report generation due to missing objectives." << '\n';
        }
    } catch (const std::exception &ex) {
        std::cerr << "Failed to initialize NSGA-II parameters: " << ex.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

