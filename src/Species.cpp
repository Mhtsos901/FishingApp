#include "../include/FishingEngine/Species.hpp"
#include <cmath>
#include <limits>

FishSpecies Species::GetSpeciesEnum(const std::string& nameStr) {
    if (nameStr == "grivadi") return FishSpecies::Carp;
    if (nameStr == "petalouda") return FishSpecies::Petalouda;
    return FishSpecies::Unknown;
}

Species::Species(FishSpecies currentSpecies) {

    if (currentSpecies == FishSpecies::Carp) {
        rules["Temperature"] = Rule{
            .type = RuleType::Continuous,
            .idealValues = {23.0},
            .toleranceBelow = 6.0,
            .toleranceAbove = 7.0,
            .weight = 0.35
        };

        rules["Pressure"] = Rule{
            .type = RuleType::Continuous,
            .idealValues = {1011.0},
            .toleranceBelow = 12.0,
            .toleranceAbove = 6.0,
            .weight = 0.25,
        };

        rules["WindDirection"] = Rule{
            .type = RuleType::Categorical,
            .weight = 0.15,
            .lutScores = {0.2, 0.1, 0.25, 0.75, 1.0, 0.8, 0.6, 0.3}
        };

        rules["TimeOfDone"] = Rule{
            .type = RuleType::Cyclical,
            .idealValues = {0.0, 0.0},
            .toleranceBelow = 2.0,
            .toleranceAbove = 3.5,
            .weight = 0.15,
            .cycleMax = 24.0
        };

        rules["WindSpeed"] = Rule{
            .type = RuleType::Continuous,
            .idealValues = {12.0},
            .toleranceBelow = 10.0,
            .toleranceAbove = 15.0,
            .weight = 0.05,
        };

        rules["Precipitation"] = Rule{
            .type = RuleType::Continuous,
            .idealValues = {2.0},
            .toleranceBelow = 4.0,
            .toleranceAbove = 5.0,
            .weight = 0.05,
        };


    }

    /*if (currentSpecies == FishSpecies::Carp) {
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
    }*/

}

double Species::calculateScore(const std::unordered_map<std::string, double>& currentWeather) const {
    double totalScore = 0.0;
    double weightSum = 0.0;

    for (const auto& [parameterName, currentRule] : rules) {
        auto it = currentWeather.find(parameterName);
        // Αν δεν βρεθεί το δεδομένο στον τρέχοντα καιρό, το προσπερνάμε
        if (it == currentWeather.end()) continue;

        const double currentValue = it->second;
        double parameterScore = 0.0; // Αποθηκεύει το σκορ (0.0 - 1.0) του τρέχοντος παράγοντα

        // --- ΜΟΝΟΠΑΤΙ 1: Κατηγορικά Δεδομένα (Lookup Table) ---
        if (currentRule.type == RuleType::Categorical) {

            // Ειδικός χειρισμός για την Κατεύθυνση Ανέμου (WindDirection)
            if (parameterName == "WindDirection") {
                // Μετατροπή μοιρών (0-360) σε δείκτη πίνακα (Index 0-7)
                int index = static_cast<int>(std::floor((currentValue + 22.5) / 45.0)) % 8;

                // Ασφαλής ανάγνωση από το array (Bounds Checking)
                if (index >= 0 && index < currentRule.lutScores.size()) {
                    parameterScore = currentRule.lutScores[index];
                }
            }
        }

        // --- ΜΟΝΟΠΑΤΙ 2: Συνεχή & Κυκλικά Δεδομένα (Asymmetrical Gaussian Falloff) ---
        else {
            double minDifference = std::numeric_limits<double>::max();
            double appliedTolerance = 0.0; // ΝΕΟ: Εδώ θα αποθηκεύσουμε το σωστό tolerance (Below ή Above)

            // Ψάχνουμε να βρούμε την πιο κοντινή ιδανική τιμή
            for (double idealValue : currentRule.idealValues) {
                double diff = std::abs(currentValue - idealValue);

                // Υποθέτουμε αρχικά ότι είμαστε κάτω από την ιδανική τιμή
                bool isBelow = (currentValue < idealValue);

                // Αν ο κανόνας είναι Κυκλικός, ελέγχουμε το wrap-around
                if (currentRule.type == RuleType::Cyclical && currentRule.cycleMax > 0.0) {
                    double alternativeDiff = currentRule.cycleMax - diff;
                    if (alternativeDiff < diff) {
                        diff = alternativeDiff;
                        // Αν η πιο κοντινή διαδρομή είναι μέσω της αλλαγής του κύκλου,
                        // η έννοια του "πάνω" και "κάτω" αντιστρέφεται!
                        isBelow = !isBelow;
                    }
                }

                // Αν βρήκαμε μια καλύτερη (πιο κοντινή) ιδανική τιμή, την κρατάμε
                if (diff < minDifference) {
                    minDifference = diff;
                    // ΝΕΟ: Επιλέγουμε δυναμικά το Tolerance βάσει της κατεύθυνσης
                    appliedTolerance = isBelow ? currentRule.toleranceBelow : currentRule.toleranceAbove;
                }
            }

            // Υπολογισμός Gauss με τη Δυναμική Ανοχή (appliedTolerance)
            if (appliedTolerance > 0.0) {
                double diffSquared = minDifference * minDifference;
                double toleranceSquared = appliedTolerance * appliedTolerance;
                parameterScore = std::exp(-diffSquared / (2.0 * toleranceSquared));
            } else {
                // Αν η ανοχή είναι 0, δίνουμε 100% μόνο αν πετύχαμε ακριβώς την ιδανική τιμή
                parameterScore = (minDifference == 0.0) ? 1.0 : 0.0;
            }
        }

        // --- ΤΕΛΙΚΟ ΒΗΜΑ: Σταθμισμένη Πρόσθεση (Weighted Addition) ---
        totalScore += parameterScore * currentRule.weight;
        weightSum += currentRule.weight;
    }

    // Επιστρέφουμε τον μέσο όρο (Weighted Average), ελέγχοντας ξανά για διαίρεση με το 0
    return (weightSum > 0.0) ? (totalScore / weightSum) : 0.0;
}

void Species::updateRuleIdealValues(const std::string& parameterName, const std::vector<double>& newIdealValues) {
    if (rules.contains(parameterName)) {
            rules[parameterName].idealValues = newIdealValues;
    }
}
