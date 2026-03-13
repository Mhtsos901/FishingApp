#pragma once

#include <string>
#include <unordered_map>

struct ParameterRule {
    double idealValue, tolerance, weight;
};

class Species {
private:
    std::string name;
    std::unordered_map<std::string, ParameterRule> rules;

public:
    Species(const std::string &fishName);

    double calculateScore(const std::unordered_map<std::string, double>& currentWeather);
};