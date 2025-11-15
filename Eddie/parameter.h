#ifndef EDDIE_PARAMETER_H
#define EDDIE_PARAMETER_H

#include <cstddef>
#include <string>
#include <vector>

struct OptimizationParameters {
    std::string problem_name{};

    std::size_t population_size = 100;
    std::size_t offspring_population_size = 100;
    std::size_t max_generations = 250;

    double crossover_probability = 0.9;
    double mutation_probability = 0.1;
    double distribution_index_crossover = 15.0;
    double distribution_index_mutation = 20.0;

    double crossover_distribution_index = 15.0; // legacy alias
    double mutation_distribution_index = 20.0;  // legacy alias

    unsigned int random_seed = 42U;

    std::vector<double> variable_lower_bounds{};
    std::vector<double> variable_upper_bounds{};

    std::vector<std::string> objective_names{};
    std::vector<std::string> variable_names{};
};

OptimizationParameters load_parameters_from_file(const std::string &path);
OptimizationParameters load_parameters_from_cli(int argc, char **argv);

#endif // EDDIE_PARAMETER_H
