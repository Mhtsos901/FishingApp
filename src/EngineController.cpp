#include "../include/FishingEngine/EngineController.hpp"
#include <QString>

EngineController::EngineController(QObject *parent) : QObject(parent), m_currentFishId(0) {
    // Αρχικοποιούμε το Service και συνδέουμε τα Signals του με τα δικά μας Slots
    m_weatherService = new WeatherService(this);

    connect(m_weatherService, &WeatherService::weatherDataReady, this, &EngineController::onWeatherReady);
    connect(m_weatherService, &WeatherService::errorOccurred, this, &EngineController::onWeatherError);
}

void EngineController::calculateCatchProbability(int locationId, int fishId) {
    m_currentFishId = fishId; // Αποθήκευση επιλογής στη μνήμη της κλάσης

    double lat = 0.0, lon = 0.0;
    if (locationId == 1) { lat = 38.56; lon = 21.47; } // Τριχωνίδα
    else if (locationId == 2) { lat = 38.74; lon = 21.18; } // Ρίβιο
    else if (locationId == 3) { lat = 38.65; lon = 21.23; } // Οζερός
    else {
        emit calculationError("Σφάλμα: Άγνωστη τοποθεσία!");
        return;
    }

    // Η κλήση φεύγει στο background (Non-blocking)
    m_weatherService->getLiveWeather(lat, lon);
}

void EngineController::onWeatherReady(const std::unordered_map<std::string, double>& weatherData) {
    if (weatherData.empty()) {
        emit calculationError("Σφάλμα: Δεν βρέθηκαν δεδομένα καιρού.");
        return;
    }

    Species* targetFish = nullptr;
    if (m_currentFishId == 1) {
        targetFish = new Species("grivadi");
        if (weatherData.contains("Sunrise") && weatherData.contains("Sunset")) {
            targetFish->updateRuleIdealValues("TimeZone", {weatherData.at("Sunrise"), weatherData.at("Sunset")});
        }
    } else if (m_currentFishId == 2) {
        targetFish = new Species("petalouda");
    } else {
        emit calculationError("Σφάλμα: Άγνωστο ψάρι!");
        return;
    }

    // Μαθηματικοί Υπολογισμοί
    const double rawScore = targetFish->calculateScore(weatherData);
    const double finalPercentage = rawScore * 100.0;

    // --- ΝΕΟ: Αποθηκεύουμε τις μοίρες και ενημερώνουμε το UI ---
    m_windDegrees = weatherData.at("RawWindDirection");
    emit windDegreesChanged(); // <--- Πολύ σημαντικό!

    // --- ΝΕΟΣ ΚΩΔΙΚΑΣ: Μετατροπή μοιρών αέρα και χτίσιμο του νέου String ---

    // Helper function (Lambda) για μετατροπή μοιρών σε κείμενο πυξίδας
    auto getCompassDirection = [](double degrees) -> QString {
        if (degrees >= 337.5 || degrees < 22.5) return "Βόρειος (N)";
        if (degrees >= 22.5 && degrees < 67.5) return "Βορειοανατολικός (NE)";
        if (degrees >= 67.5 && degrees < 112.5) return "Ανατολικός (E)";
        if (degrees >= 112.5 && degrees < 157.5) return "Νοτιοανατολικός (SE)";
        if (degrees >= 157.5 && degrees < 202.5) return "Νότιος (S)";
        if (degrees >= 202.5 && degrees < 247.5) return "Νοτιοδυτικός (SW)";
        if (degrees >= 247.5 && degrees < 292.5) return "Δυτικός (W)";
        if (degrees >= 292.5 && degrees < 337.5) return "Βορειοδυτικός (NW)";
        return "Άγνωστη";
    };

    QString compassDir = getCompassDirection(weatherData.at("RawWindDirection"));

    QString debugInfo = QString("Θερμοκ. Νερού: %1 °C  |  Αέρα: %2 °C\nΑέρας: %3 km/h  [%4]\nΒροχή: %5 mm\nΒαρόμετρο: %6 hPa")
                        .arg(weatherData.at("Temperature"), 0, 'f', 1)
                        .arg(weatherData.at("AirTemperature"), 0, 'f', 1)
                        .arg(weatherData.at("WindSpeed"), 0, 'f', 1)
                        .arg(compassDir)
                        .arg(weatherData.at("Precipitation"), 0, 'f', 1)
                        .arg(weatherData.at("Pressure"), 0, 'f', 1);

    // -------------------------------------------------------------------------

    // ΣΤΕΛΝΟΥΜΕ ΤΑ ΔΕΔΟΜΕΝΑ ΣΤΟ UI!
    emit calculationFinished(finalPercentage, debugInfo);

    delete targetFish; // Αποφυγή Memory Leak
}

void EngineController::onWeatherError(const std::string& errorMsg) {
    emit calculationError(QString::fromStdString(errorMsg));
}