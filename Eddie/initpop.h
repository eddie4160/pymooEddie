#ifndef EDDIE_INITPOP_H
#define EDDIE_INITPOP_H

#include <random>
#include <vector>

#include "parameter.h"

using Individual = std::vector<double>;
using Population = std::vector<Individual>;

Population latin_hypercube_population(const OptimizationParameters &params, std::mt19937 &rng);

Population latin_hypercube_population(const OptimizationParameters &params);

#endif // EDDIE_INITPOP_H
