#include "../include/FishingEngine/EngineController.hpp"
#include "../include/FishingEngine/WeatherUtils.hpp"
#include <QString>
#include <QDate>
#include <cmath>

EngineController::EngineController(QObject *parent) : QObject(parent), m_currentFishId(0) {
    // Αρχικοποιούμε το Service και συνδέουμε τα Signals του με τα δικά μας Slots
    m_weatherService = new WeatherService(this);

    connect(m_weatherService, &WeatherService::weatherDataReady, this, &EngineController::onWeatherReady);
    connect(m_weatherService, &WeatherService::errorOccurred, this, &EngineController::onWeatherError);
}

void EngineController::calculateCatchProbability(int locationId, int fishId) {
    m_currentFishId = fishId; // Αποθήκευση επιλογής στη μνήμη της κλάσης
    m_currentLocationId = locationId;

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

    // 1. ΥΠΟΛΟΓΙΣΜΟΣ ΕΠΙΦΑΝΕΙΑΣ (Επιλίμνιο)
    double surfaceTemp = weatherData.at("Temperature");
    const double surfaceScore = targetFish->calculateScore(weatherData);
    const double surfacePct = surfaceScore * 100.0;

    // 2. ΡΥΘΜΙΣΕΙΣ ΛΙΜΝΗΣ & ΥΠΟΛΟΓΙΣΜΟΣ ΘΕΡΜΟΚΛΙΝΑΣ
    double maxDepth = 10.0; // Προεπιλογή (Οζερός)
    if (m_currentLocationId == 1) maxDepth = 58.0; // Τριχωνίδα
    else if (m_currentLocationId == 2) maxDepth = 40.0; // Ρίβιο

    int currentMonth = QDate::currentDate().month();
    double windKmh = weatherData.at("WindSpeed");
    double z_th = WeatherUtils::calculateThermoclineDepth(currentMonth, maxDepth, windKmh);

    // 3. ΑΝΑΖΗΤΗΣΗ ΙΔΑΝΙΚΟΥ ΒΑΘΟΥΣ (Linear Search Algorithm)
    double bestThermoPct = 0.0;
    double bestDepth = 0.0;
    double bestTemp = surfaceTemp;

    // Σαρώνουμε από τη θερμοκλίνα μέχρι τον πάτο για να βρούμε την τέλεια θερμοκρασία
    if (z_th > 0.0 && z_th < maxDepth) {
        double maxScoreFound = -1.0;
        // Χρησιμοποιούμε std::ceil για να στρογγυλοποιήσουμε στο επόμενο ακέραιο μέτρο!
        // Αν z_th = 9.9, το startDepth θα γίνει 10.
        int startDepth = static_cast<int>(std::ceil(z_th));
        int endDepth = static_cast<int>(maxDepth);

        for (int currentDepth = startDepth; currentDepth <= endDepth; ++currentDepth) {
            double tempAtDepth = WeatherUtils::calculateTempAtDepth(surfaceTemp, z_th, currentDepth);

            // Φτιάχνουμε προσωρινό map δεδομένων αλλάζοντας μόνο τη θερμοκρασία
            auto testWeatherData = weatherData;
            testWeatherData["Temperature"] = tempAtDepth;

            double testScore = targetFish->calculateScore(testWeatherData);

            // Κρατάμε το μέγιστο (Maximization)
            if (testScore > maxScoreFound) {
                maxScoreFound = testScore;
                bestDepth = currentDepth;
                bestTemp = tempAtDepth;
            }
        }
        bestThermoPct = maxScoreFound * 100.0;
    } else {
        // Χειμώνας ή πλήρως ανακατεμένη λίμνη
        bestThermoPct = surfacePct;
        bestDepth = 0.0;
    }

    // 4. ΕΤΟΙΜΑΣΙΑ ΤΟΥ UI & ΣΗΜΑΤΩΝ (Signals)
    m_windDegrees = weatherData.at("RawWindDirection");
    emit windDegreesChanged();

    // Helper function για την Πυξίδα
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

    // Helper function (Lambda) για μετατροπή km/h σε Μποφόρ
    auto getBeaufort = [](double kmh) -> int {
        if (kmh < 2.0) return 0;
        if (kmh < 6.0) return 1;
        if (kmh < 12.0) return 2;
        if (kmh < 20.0) return 3;
        if (kmh < 29.0) return 4;
        if (kmh < 39.0) return 5;
        if (kmh < 50.0) return 6;
        if (kmh < 62.0) return 7;
        if (kmh < 75.0) return 8;
        if (kmh < 89.0) return 9;
        if (kmh < 103.0) return 10;
        if (kmh < 118.0) return 11;
        return 12;
    };

    QString compassDir = getCompassDirection(m_windDegrees);
    int beaufort = getBeaufort(windKmh);
    double airTemp = weatherData.at("AirTemperature");

    // --- Χτίζουμε το κείμενο με τον έλεγχο για το Επιλίμνιο ---
    QString debugInfo;

    if (z_th > 0.0) {
        debugInfo = QString("Βάθος Επιλιμνίου: 0 έως %1 m\nΘερμ. Νερού (Επιφάνεια): %2 °C\nΘερμ. Νερού (%3m): %4 °C\nΘερμ. Αέρα: %5 °C\nΑέρας: %6 Μποφόρ - %7 km/h [%8]\nΒροχή: %9 mm\nΒαρόμετρο: %10 hPa")
                            .arg(z_th, 0, 'f', 1)
                            .arg(surfaceTemp, 0, 'f', 1)
                            .arg(bestDepth, 0, 'f', 0)
                            .arg(bestTemp, 0, 'f', 1)
                            .arg(airTemp, 0, 'f', 1)
                            .arg(beaufort)
                            .arg(windKmh, 0, 'f', 1)
                            .arg(compassDir)
                            .arg(weatherData.at("Precipitation"), 0, 'f', 1)
                            .arg(weatherData.at("Pressure"), 0, 'f', 1);
    } else {
        debugInfo = QString("Βάθος Επιλιμνίου: Η λίμνη είναι ανακατεμένη\nΘερμ. Νερού (Επιφάνεια): %1 °C\nΘερμ. Αέρα: %2 °C\nΑέρας: %3 Μποφόρ - %4 km/h [%5]\nΒροχή: %6 mm\nΒαρόμετρο: %7 hPa")
                            .arg(surfaceTemp, 0, 'f', 1)
                            .arg(airTemp, 0, 'f', 1)
                            .arg(beaufort)
                            .arg(windKmh, 0, 'f', 1)
                            .arg(compassDir)
                            .arg(weatherData.at("Precipitation"), 0, 'f', 1)
                            .arg(weatherData.at("Pressure"), 0, 'f', 1);
    }

    // ΣΤΕΛΝΟΥΜΕ 4 ΔΕΔΟΜΕΝΑ ΣΤΟ UI!
    emit calculationFinished(surfacePct, bestThermoPct, bestDepth, debugInfo);

    delete targetFish; // Αποφυγή Memory Leak!
}

void EngineController::onWeatherError(const std::string& errorMsg) {
    emit calculationError(QString::fromStdString(errorMsg));
}