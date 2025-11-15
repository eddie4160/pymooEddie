#include "problem.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <stdexcept>
#include <string>

namespace {
constexpr double pi() {
    return 3.14159265358979323846;
}
} // namespace

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

std::string canonicalize_problem_name(const std::string &name) {
    std::string canonical = name;
    canonical.erase(std::remove_if(canonical.begin(), canonical.end(), [](unsigned char ch) {
                          return std::isspace(ch) != 0;
                      }),
                      canonical.end());
    std::transform(canonical.begin(), canonical.end(), canonical.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return canonical;
}

bool is_problem_recorded(const std::string &name) {
    const std::string canonical = canonicalize_problem_name(name);
    return canonical == "ZDT4" || canonical == "CFD";
}

bool has_problem_implementation(const std::string &name) {
    const std::string canonical = canonicalize_problem_name(name);
    return canonical == "ZDT4";
}

std::vector<std::vector<double>> evaluate_problem_population(const std::string &name, const Population &population) {
    const std::string canonical = canonicalize_problem_name(name);
    if (canonical == "ZDT4") {
        return evaluate_zdt4_population(population);
    }
    if (canonical == "CFD") {
        throw std::runtime_error("CFD evaluation is not implemented in this demo.");
    }

    throw std::invalid_argument("Problem '" + name + "' is not recorded in problem.cpp.");
}

