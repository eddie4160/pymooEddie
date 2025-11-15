#include <cstdlib>
#include <exception>
#include <iostream>

#include "parameter.h"

namespace {

eddie::OptimizationParameters load_default_parameters() {
    eddie::OptimizationParameters params{};

    params.variable_names = {"x1", "x2", "x3", "x4", "x5"};
    params.objective_names = {"Objective 1", "Objective 2"};

    params.variable_lower_bounds = {0.0, 0.0, 0.0, 0.0, 0.0};
    params.variable_upper_bounds = {1.0, 1.0, 1.0, 1.0, 1.0};

    return params;
}

void print_parameters(const eddie::OptimizationParameters &params) {
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

} // namespace

int main() {
    try {
        const auto params = load_default_parameters();
        print_parameters(params);
    } catch (const std::exception &ex) {
        std::cerr << "Failed to initialize NSGA-II parameters: " << ex.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
