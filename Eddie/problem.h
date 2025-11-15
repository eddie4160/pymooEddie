#ifndef EDDIE_PROBLEM_H
#define EDDIE_PROBLEM_H

#include <string>
#include <vector>

#include "initpop.h"

std::vector<double> evaluate_zdt4(const Individual &decision_vector);
std::vector<std::vector<double>> evaluate_zdt4_population(const Population &population);

std::string canonicalize_problem_name(const std::string &name);
bool is_problem_recorded(const std::string &name);
bool has_problem_implementation(const std::string &name);
std::vector<std::vector<double>> evaluate_problem_population(const std::string &name, const Population &population);

#endif // EDDIE_PROBLEM_H
