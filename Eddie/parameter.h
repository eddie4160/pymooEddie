#ifndef EDDIE_PARAMETER_H
#define EDDIE_PARAMETER_H

#include <cstddef>
#include <string>
#include <vector>

namespace eddie {

struct OptimizationParameters {
    std::size_t population_size = 100;
    std::size_t offspring_population_size = 100;
    std::size_t max_generations = 250;
    double crossover_probability = 0.9;
    double mutation_probability = 0.1;
    double distribution_index_crossover = 15.0;
    double distribution_index_mutation = 20.0;
    unsigned int random_seed = 42U;

    std::vector<double> variable_lower_bounds{};
    std::vector<double> variable_upper_bounds{};
    std::vector<std::string> objective_names{};
    std::vector<std::string> variable_names{};
};

OptimizationParameters load_parameters_from_file(const std::string &path);

} // namespace eddie

#endif // EDDIE_PARAMETER_H
