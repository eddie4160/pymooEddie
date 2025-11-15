#include <algorithm>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>

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

int main() {
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
        }
    } catch (const std::exception &ex) {
        std::cerr << "Failed to initialize NSGA-II parameters: " << ex.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
