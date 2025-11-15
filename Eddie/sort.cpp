#include "sort.h"

#include <stdexcept>
#include <vector>

namespace {
bool dominates(const std::vector<double> &lhs, const std::vector<double> &rhs) {
    if (lhs.size() != rhs.size()) {
        throw std::invalid_argument("Objective vectors must have identical dimensions for comparison");
    }

    bool strictly_better = false;
    for (std::size_t i = 0; i < lhs.size(); ++i) {
        if (lhs[i] > rhs[i]) {
            return false;
        }
        if (lhs[i] < rhs[i]) {
            strictly_better = true;
        }
    }

    return strictly_better;
}
}

NonDominatedSortResult fast_non_dominated_sort(const std::vector<std::vector<double>> &objectives) {
    const std::size_t population_size = objectives.size();
    NonDominatedSortResult result{};
    result.fronts.clear();
    result.ranks.assign(population_size, 0);

    if (population_size == 0) {
        return result;
    }

    std::vector<std::vector<std::size_t>> dominates_set(population_size);
    std::vector<std::size_t> domination_count(population_size, 0);

    for (std::size_t p = 0; p < population_size; ++p) {
        for (std::size_t q = 0; q < population_size; ++q) {
            if (p == q) {
                continue;
            }

            if (dominates(objectives[p], objectives[q])) {
                dominates_set[p].push_back(q);
            } else if (dominates(objectives[q], objectives[p])) {
                ++domination_count[p];
            }
        }

        if (domination_count[p] == 0) {
            result.ranks[p] = 0;
        }
    }

    std::vector<std::size_t> first_front;
    for (std::size_t i = 0; i < population_size; ++i) {
        if (domination_count[i] == 0) {
            first_front.push_back(i);
        }
    }

    if (first_front.empty()) {
        throw std::runtime_error("Non-dominated sorting failed: first front is empty");
    }

    result.fronts.push_back(first_front);

    std::size_t current_rank = 0;
    while (current_rank < result.fronts.size()) {
        std::vector<std::size_t> next_front;
        for (const auto individual_index : result.fronts[current_rank]) {
            for (const auto dominated_index : dominates_set[individual_index]) {
                if (domination_count[dominated_index] == 0) {
                    continue;
                }

                --domination_count[dominated_index];
                if (domination_count[dominated_index] == 0) {
                    result.ranks[dominated_index] = current_rank + 1;
                    next_front.push_back(dominated_index);
                }
            }
        }

        if (!next_front.empty()) {
            result.fronts.push_back(next_front);
        }

        ++current_rank;
    }

    return result;
}
