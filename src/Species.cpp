#include "../include/FishingEngine/Species.hpp"
#include <cmath>

FishSpecies Species::GetSpeciesEnum(const std::string& nameStr) {
    if (nameStr == "grivadi") return FishSpecies::Carp;
    if (nameStr == "petalouda") return FishSpecies::Petalouda;
    return FishSpecies::Unknown;
}

Species::Species(const std::string &fishName) {
    name = fishName;

    FishSpecies currentSpecies = GetSpeciesEnum(fishName);

    if (currentSpecies == FishSpecies::Carp) {
        rules["Temperature"] = {{20.0}, 5.0, 0.35};
        rules["Pressure"] = {{1005.0}, 13.0, 0.25};
        rules["WindDirection"] = {{100.0}, 40.0, 0.15};
        rules["TimeZone"] = {{0.0, 0.0}, 6.0, 0.15};
        rules["WindSpeed"] = {{9.0}, 12.0, 0.05};
        rules["Precipitation"] = {{2.0}, 4.0, 0.05};

    }
    else if (currentSpecies == FishSpecies::Petalouda) {
        rules["Temperature"] = {{21.5}, 8.5, 0.35};
        rules["Pressure"] = {{1013.0}, 10.0, 0.20};
        rules["WindDirection"] = {{100.0}, 50.0, 0.10};
        rules["TimeZone"] = {{0.0, 0.0}, 6.0, 0.15};
        rules["WindSpeed"] = {{10.0}, 15.0, 0.05};
        rules["Precipitation"] = {{1.5}, 5.0, 0.05};
    }

}

double Species::calculateScore(const std::unordered_map<std::string, double>& currentWeather) const {
    double totalScore = 0.0;
    double weightSum = 0.0;

    for (const auto& [parameterName, currentRule] : rules) {
        auto it = currentWeather.find(parameterName);
        if (it == currentWeather.end()) continue;

        const double currentValue = it->second;
        double minDifference = std::numeric_limits<double>::max();

        for (double idealValue : currentRule.idealValues) {
            double diff = std::abs(currentValue - idealValue);
            if (diff < minDifference) {
                minDifference = diff;
            }
        }

        const double parameterScore = std::exp(
            -std::pow(minDifference, 2) / (2 * std::pow(currentRule.tolerance, 2))
        );

        totalScore += parameterScore * currentRule.weight;
        weightSum += currentRule.weight;
    }

    return (weightSum > 0.0) ? totalScore / weightSum : 0.0;
}

void Species::updateRuleIdealValues(const std::string& parameterName, const std::vector<double>& newIdealValues) {
    if (rules.contains(parameterName)) {
            rules[parameterName].idealValues = newIdealValues;
    }
}
