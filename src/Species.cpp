#include "../include/FishingEngine/Species.hpp"
#include <cmath>

Species::Species(const std::string &fishName) {
    name = fishName;

    if (name == "grivadi") {
        rules["Temperature"] = {{20.0}, 5.0, 0.35};
        rules["Pressure"] = {{1010.0}, 13.0, 0.25};
        rules["WindDirection"] = {{100.0}, 40.0, 0.15};
        rules["WindSpeed"] = {{13.0}, 12.0, 0.1};
        rules["TimeZone"] = {{0.0, 0.0}, 6.0, 0.1};
        rules["Precipitation"] = {{1.3}, 6.0, 0.05};

    }
    else if (name == "petalouda") {
        rules["Temperature"] = {{22.0}, 6.0, 0.45};
        rules["WindDirection"] = {{100.0}, 40.0, 0.2};
        rules["Pressure"] = {{1005.0}, 12.0, 0.15};
        rules["Photoperiod"] = {{14.0}, 3.0, 0.08};
        rules["WindSpeed"] = {{12.0}, 10.0, 0.12};
    }

}

double Species::calculateScore(const std::unordered_map<std::string, double>& currentWeather) {
    double totalScore = 0.0;

    for (const auto& [parameterName, currentValue] : currentWeather) {
        if (rules.contains(parameterName)) {
            const auto& currentRule = rules[parameterName];

            double minDifference = std::numeric_limits<double>::max();

            for (double idealValue : currentRule.idealValues) {
                double currentDifference = std::abs(currentValue - idealValue);
                if (currentDifference < minDifference) {
                    minDifference = currentDifference;
                }
            }

            const double parameterScore = std::exp(-std::pow(minDifference, 2) / (2 * std::pow(currentRule.tolerance, 2)));

            totalScore += parameterScore * currentRule.weight;
        }
    }
    return totalScore;
}

void Species::updateRuleIdealValues(const std::string& parameterName, const std::vector<double>& newIdealValues) {
    if (rules.contains(parameterName)) {
            rules[parameterName].idealValues = newIdealValues;
    }
}
