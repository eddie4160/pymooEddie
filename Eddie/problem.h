#ifndef EDDIE_PROBLEM_H
#define EDDIE_PROBLEM_H

#include <vector>

#include "initpop.h"

std::vector<double> evaluate_zdt4(const Individual &decision_vector);

std::vector<std::vector<double>> evaluate_zdt4_population(const Population &population);

#endif // EDDIE_PROBLEM_H
