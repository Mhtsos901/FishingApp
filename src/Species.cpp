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
            .weight = 0.45
        };

        rules["Pressure"] = Rule{
            .type = RuleType::Continuous,
            .idealValues = {1011.0},
            .toleranceBelow = 12.0,
            .toleranceAbove = 6.0,
            .weight = 0.15,
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
}

// ΠΡΟΣΟΧΗ: Αλλάξαμε τον τύπο επιστροφής από double σε ScoreDetails
ScoreDetails Species::calculateScore(const std::unordered_map<std::string, double>& currentWeather) const {
    ScoreDetails result; // ΝΕΟ: Φτιάχνουμε τον άδειο "Φάκελο" Διαγνωστικών
    double totalScore = 0.0;
    double weightSum = 0.0;

    for (const auto& [parameterName, currentRule] : rules) {

        weightSum += currentRule.weight;

        auto it = currentWeather.find(parameterName);
        // Αν δεν βρεθεί το δεδομένο στον τρέχοντα καιρό, το προσπερνάμε
        if (it == currentWeather.end()) {
            result.parameterScores[parameterName] = 0.0;
            continue;
        }
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
            double appliedTolerance = 0.0; // Εδώ θα αποθηκεύσουμε το σωστό tolerance (Below ή Above)

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
                    // Επιλέγουμε δυναμικά το Tolerance βάσει της κατεύθυνσης
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

        // --- ΝΕΟ: Αποθηκεύουμε το επιμέρους σκορ στον Φάκελο (Map) ---
        result.parameterScores[parameterName] = parameterScore;

        // --- ΤΕΛΙΚΟ ΒΗΜΑ: Σταθμισμένη Πρόσθεση (Weighted Addition) ---
        totalScore += parameterScore * currentRule.weight;
    }

    // Υπολογίζουμε το συνολικό σκορ (Weighted Average) και το βάζουμε στον Φάκελο
    result.totalScore = (weightSum > 0.0) ? (totalScore / weightSum) : 0.0;

    // Επιστρέφουμε ολόκληρο το αντικείμενο!
    return result;
}

void Species::updateRuleIdealValues(const std::string& parameterName, const std::vector<double>& newIdealValues) {
    if (rules.contains(parameterName)) {
            rules[parameterName].idealValues = newIdealValues;
    }
}
