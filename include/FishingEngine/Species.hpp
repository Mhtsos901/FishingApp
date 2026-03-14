#pragma once

#include <string>
#include <unordered_map>
#include <vector>

struct RuleDefinition {
    std::vector<double> idealValues;
    double tolerance;
    double weight;
};



class Species {
private:
    std::string name;
    std::unordered_map<std::string, RuleDefinition> rules;

public:
    Species(const std::string& fishName);

    double calculateScore(const std::unordered_map<std::string, double>& currentWeather) const;

    void updateRuleIdealValues(const std::string& parameterName, const std::vector<double>& newIdealValues);
};