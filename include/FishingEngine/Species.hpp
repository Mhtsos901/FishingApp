#pragma once

#include <vector>
#include <string>
#include <unordered_map>

// ΝΕΟ: Ορίζουμε τους τύπους συμπεριφοράς των δεδομένων
enum class RuleType {
    Continuous,  // Γραμμικά δεδομένα (π.χ. Θερμοκρασία, Πίεση)
    Cyclical,    // Κυκλικά δεδομένα (π.χ. Ώρα της ημέρας 0-24)
    Categorical  // Κατηγορικά/Διακριτά δεδομένα (π.χ. Άνεμος μέσω LUT)
};

// Το ΜΟΝΑΔΙΚΟ struct που χρειαζόμαστε πλέον για τους κανόνες
struct Rule {
    RuleType type = RuleType::Continuous; // Προεπιλογή

    // -- Για Continuous & Cyclical (Gaussian Logic) --
    std::vector<double> idealValues;
    double toleranceBelow = 0.0;
    double toleranceAbove = 0.0;
    double weight = 0.0; // Η βαρύτητα (Weight) αφορά όλους τους κανόνες!
    double cycleMax = 0.0; // π.χ. 24.0 για την ώρα (Κυκλικά)

    // -- Για Categorical (Lookup Table Logic) --
    std::vector<double> lutScores; // Ποσοστά για διακριτά δεδομένα (π.χ. 8 οκτημόρια αέρα)
};

enum class FishSpecies {
    Carp,
    Petalouda,
    Unknown
};

class Species {
private:
    std::string name;

    // ΕΔΩ Η ΔΙΟΡΘΩΣΗ: Το map πλέον χρησιμοποιεί το νέο struct 'Rule'
    std::unordered_map<std::string, Rule> rules;

    FishSpecies GetSpeciesEnum(const std::string& nameStr);

public:
    Species(FishSpecies species);

    double calculateScore(const std::unordered_map<std::string, double>& currentWeather) const;

    void updateRuleIdealValues(const std::string& parameterName, const std::vector<double>& newIdealValues);
};