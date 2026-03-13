#include "../include/FishingEngine/Species.hpp"
#include <cmath>

Species::Species(const std::string &fishName) {
    name = fishName;

    if (name == "grivadi") {
        rules["Temperature"] = {20.0, 3.0, 0.45};
        rules["WindDirection"] = {100.0, 25.0, 0.25};
        rules["Pressure"] = {1010.0, 5.0, 0.12};
        rules["Photoperiod"] = {13.0,2.0, 0.05};
        rules["WindSpeed"] = {15.0, 12.0, 0.13};
    }
    else if (name == "petalouda") {
        rules["Temperature"] = {22.0, 6.0, 0.45};
        rules["WindDirection"] = {100.0, 40.0, 0.2};
        rules["Pressure"] = {1005.0, 12.0, 0.15};
        rules["Photoperiod"] = {14.0, 3.0, 0.08};
        rules["WindSpeed"] = {12.0, 10.0, 0.12};
    }

}

double Species::calculateScore(const std::unordered_map<std::string, double>& currentWeather){
    double totalScore = 0.0;

    for (const auto& [parameterName, currentValue] : currentWeather) {
        if (rules.count(parameterName) > 0) {
            ParameterRule rule = rules[parameterName];

            double difference = currentValue - rule.idealValue;
            double parameterScore = std::exp(-std::pow(difference, 2) / (2 * std::pow(rule.tolerance, 2)));

            totalScore += parameterScore * rule.weight;
        }
    }
    return totalScore;
}