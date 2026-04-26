#pragma once

#include <vector>
#include <string>
#include <unordered_map>

enum class RuleType {
    Continuous,
    Cyclical,
    Categorical
};

struct Rule {
    RuleType type = RuleType::Continuous;

    // Continuous & Cyclical Gaussian Logic
    std::vector<double> idealValues;
    double toleranceBelow = 0.0;
    double toleranceAbove = 0.0;
    double weight = 0.0;
    double cycleMax = 0.0;

    std::vector<double> lutScores;
};

enum class FishSpecies {
    Carp,
    Petalouda,
    Unknown
};

struct ScoreDetails {
    double totalScore = 0.0;
    std::unordered_map<std::string, double> parameterScores; // Κρατάει το επιμέρους σκορ για κάθε παράμετρο
};

class Species {
private:
    std::string name;

    std::unordered_map<std::string, Rule> rules;

    FishSpecies GetSpeciesEnum(const std::string& nameStr);

public:
    Species(FishSpecies species);

    ScoreDetails calculateScore(const std::unordered_map<std::string, double>& currentWeather) const;

    void updateRuleIdealValues(const std::string& parameterName, const std::vector<double>& newIdealValues);
};