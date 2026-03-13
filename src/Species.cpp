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
        if (rules.contains(parameterName)) {
            auto [idealValue, tolerance, weight] = rules[parameterName];

            const double difference = currentValue - idealValue;
            const double parameterScore = std::exp(-std::pow(difference, 2) / (2 * std::pow(tolerance, 2)));

            totalScore += parameterScore * weight;
        }
    }
    return totalScore;
}