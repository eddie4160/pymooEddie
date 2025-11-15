#ifndef EDDIE_SORT_H
#define EDDIE_SORT_H

#include <cstddef>
#include <vector>

struct NonDominatedSortResult {
    std::vector<std::vector<std::size_t>> fronts;
    std::vector<std::size_t> ranks;
};

NonDominatedSortResult fast_non_dominated_sort(const std::vector<std::vector<double>> &objectives);

#endif // EDDIE_SORT_H
