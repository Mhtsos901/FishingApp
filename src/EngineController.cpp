#include "../include/FishingEngine/EngineController.hpp"
#include "../include/FishingEngine/WeatherUtils.hpp"
#include <QString>
#include <QDate>
#include <cmath>
#include <memory>

EngineController::EngineController(QObject *parent) : QObject(parent) {
    // Αρχικοποιούμε το Service και συνδέουμε τα Signals του με τα δικά μας Slots
    m_weatherService = new WeatherService(this);

    connect(m_weatherService, &WeatherService::weatherDataReady, this, &EngineController::onWeatherReady);
    connect(m_weatherService, &WeatherService::errorOccurred, this, &EngineController::onWeatherError);
    m_windDegrees = 0.0;
}

// ΑΛΛΑΓΗ 1: Δέχεται QStrings (Slugs) από το QML αντί για ints
void EngineController::calculateCatchProbability(const QString& locationKey, const QString& fishKey) {
    m_currentFishKey = fishKey; // Αποθήκευση επιλογής ψαριού στη μνήμη

    // --- DATA-DRIVEN DESIGN: Το "Λεξικό" των Λιμνών ---
    static const QMap<QString, LakeData> lakes = {
        {"trichonida", {"Τριχωνίδα", 38.56, 21.47, 58.0}},
        {"rivio",      {"Ρίβιο", 38.74, 21.18, 40.0}},
        {"ozeros",     {"Οζερός", 38.65, 21.23, 10.0}}
    };

    // Έλεγχος Ασφαλείας: Υπάρχει το κλειδί που μας έστειλε το QML;
    if (!lakes.contains(locationKey)) {
        emit calculationError("Σφάλμα: Άγνωστη τοποθεσία!");
        return;
    }

    // Αποθηκεύουμε την επιλεγμένη λίμνη στο state για να τη βρει το onWeatherReady
    m_currentLake = lakes.value(locationKey);

    // Η κλήση φεύγει στο background (Non-blocking), χρησιμοποιώντας τις συντεταγμένες του struct
    m_weatherService->getLiveWeather(m_currentLake.lat, m_currentLake.lon);
}

void EngineController::onWeatherReady(const std::unordered_map<std::string, double>& weatherData) {
    if (weatherData.empty()) {
        emit calculationError("Σφάλμα: Δεν βρέθηκαν δεδομένα καιρού.");
        return;
    }

    std::unique_ptr<Species> targetFish;

    // ΑΛΛΑΓΗ 2: Αναζήτηση βάσει του String Key
    if (m_currentFishKey == "carp") {
        targetFish = std::make_unique<Species>(FishSpecies::Carp);
        if (weatherData.contains("Sunrise") && weatherData.contains("Sunset")) {
            targetFish->updateRuleIdealValues("TimeOfDay", {weatherData.at("Sunrise"), weatherData.at("Sunset")});
        }
    } else if (m_currentFishKey == "petalouda") {
        targetFish = std::make_unique<Species>(FishSpecies::Petalouda);
        if (weatherData.contains("Sunrise") && weatherData.contains("Sunset")) {
            targetFish->updateRuleIdealValues("TimeOfDay", {weatherData.at("Sunrise"), weatherData.at("Sunset")});
        }
    } else {
        emit calculationError("Σφάλμα: Άγνωστο είδος ψαριού!");
        return;
    }

    // 1. ΥΠΟΛΟΓΙΣΜΟΣ ΕΠΙΦΑΝΕΙΑΣ (Επιλίμνιο)
    double surfaceTemp = weatherData.contains("Temperature") ? weatherData.at("Temperature") : 0.0;
    const double surfaceScore = targetFish->calculateScore(weatherData);
    const double surfacePct = surfaceScore * 100.0;

    // 2. ΡΥΘΜΙΣΕΙΣ ΛΙΜΝΗΣ & ΥΠΟΛΟΓΙΣΜΟΣ ΘΕΡΜΟΚΛΙΝΑΣ
    // ΑΛΛΑΓΗ 3: Παίρνουμε το βάθος απευθείας από το m_currentLake. Τέλος τα if-else!
    double maxDepth = m_currentLake.maxDepth;

    int currentMonth = QDate::currentDate().month();
    double windKmh = weatherData.contains("WindSpeed") ? weatherData.at("WindSpeed") : 0.0;
    double z_th = WeatherUtils::calculateThermoclineDepth(currentMonth, maxDepth, windKmh);

    // 3. ΑΝΑΖΗΤΗΣΗ ΙΔΑΝΙΚΟΥ ΒΑΘΟΥΣ (Linear Search Algorithm)
    double bestThermoPct = 0.0;
    double bestDepth = 0.0;
    double bestTemp = surfaceTemp;

    if (z_th > 0.0 && z_th < maxDepth) {
        double maxScoreFound = -1.0;
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
        bestThermoPct = surfacePct;
        bestDepth = 0.0;
    }

    // 4. ΕΤΟΙΜΑΣΙΑ ΤΟΥ UI & ΣΗΜΑΤΩΝ (Model to View)
    if (weatherData.contains("WindDirection")) {
        m_windDegrees = weatherData.at("WindDirection");
    } else {
        m_windDegrees = 0.0;
    }
    emit windDegreesChanged();

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

    QVariantMap weatherStats;
    weatherStats["thermoclineDepth"] = z_th;
    weatherStats["surfaceTemp"] = surfaceTemp;
    weatherStats["bestDepth"] = bestDepth;
    weatherStats["bestTemp"] = bestTemp;
    weatherStats["airTemp"] = weatherData.contains("AirTemperature") ? weatherData.at("AirTemperature") : 0.0;
    weatherStats["beaufort"] = getBeaufort(windKmh);
    weatherStats["windKmh"] = windKmh;
    weatherStats["compassDir"] = getCompassDirection(m_windDegrees);

    // Αμυντική προσέγγιση για βροχή/πίεση
    weatherStats["rain"] = weatherData.contains("Precipitation") ? weatherData.at("Precipitation") : 0.0;
    weatherStats["pressure"] = weatherData.contains("Pressure") ? weatherData.at("Pressure") : 0.0;

    // ΣΤΕΛΝΟΥΜΕ ΤΟ QVariantMap ΣΤΟ UI!
    emit calculationFinished(surfacePct, bestThermoPct, bestDepth, weatherStats);
}

void EngineController::onWeatherError(const std::string& errorMsg) {
    emit calculationError(QString::fromStdString(errorMsg));
}