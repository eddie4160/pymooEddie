#include "problem.h"

#include <cmath>
#include <stdexcept>

namespace {
constexpr double pi() {
    return 3.14159265358979323846;
}
}

std::vector<double> evaluate_zdt4(const Individual &decision_vector) {
    if (decision_vector.size() < 2) {
        throw std::invalid_argument("ZDT4 requires at least two decision variables");
    }

    const std::size_t dimension = decision_vector.size();

    const double f1 = decision_vector.front();

    double g = 1.0 + 10.0 * static_cast<double>(dimension - 1);
    for (std::size_t i = 1; i < dimension; ++i) {
        const double xi = decision_vector[i];
        g += xi * xi - 10.0 * std::cos(4.0 * pi() * xi);
    }

    const double h = 1.0 - std::sqrt(f1 / g);
    const double f2 = g * h;

    return {f1, f2};
}

std::vector<std::vector<double>> evaluate_zdt4_population(const Population &population) {
    std::vector<std::vector<double>> objectives;
    objectives.reserve(population.size());

    for (const auto &individual : population) {
        objectives.push_back(evaluate_zdt4(individual));
    }

    return objectives;
}
