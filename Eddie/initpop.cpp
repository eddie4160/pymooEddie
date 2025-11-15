#include "initpop.h"

#include <algorithm>
#include <numeric>

namespace eddie {

Population latin_hypercube_population(const OptimizationParameters &params, std::mt19937 &rng) {
    const std::size_t population_size = params.population_size;
    const std::size_t dimension = params.variable_names.empty()
                                      ? std::max(params.variable_lower_bounds.size(),
                                                 params.variable_upper_bounds.size())
                                      : params.variable_names.size();

    Population population(population_size, Individual(dimension, 0.0));

    std::uniform_real_distribution<double> unit_dist(0.0, 1.0);

    for (std::size_t dim = 0; dim < dimension; ++dim) {
        const double lower = dim < params.variable_lower_bounds.size() ? params.variable_lower_bounds[dim] : 0.0;
        const double upper = dim < params.variable_upper_bounds.size() ? params.variable_upper_bounds[dim] : 1.0;
        std::vector<std::size_t> permutation(population_size);
        std::iota(permutation.begin(), permutation.end(), 0U);
        std::shuffle(permutation.begin(), permutation.end(), rng);

        for (std::size_t i = 0; i < population_size; ++i) {
            const double jitter = unit_dist(rng);
            const double scaled = (static_cast<double>(permutation[i]) + jitter) /
                                  static_cast<double>(population_size);
            const double value = lower + scaled * (upper - lower);
            population[i][dim] = value;
        }
    }

    return population;
}

Population latin_hypercube_population(const OptimizationParameters &params) {
    std::mt19937 rng(params.random_seed);
    return latin_hypercube_population(params, rng);
}

} // namespace eddie
