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
        {"trichonida_west", {
            .name = "Τριχωνίδα (Δυτικά)",
            .lat = 38.55,
            .lon = 21.46,
            .maxDepth = 45.0,
            .monthlyBaseTemps = {11.0, 11.5, 13.0, 15.5, 20.0, 24.5, 27.0, 28.0, 25.5, 22.0, 18.0, 13.5}
        }},
    {"trichonida_east", {
            .name = "Τριχωνίδα (Ανατολικά)",
            .lat = 38.52,
            .lon = 21.63,
            .maxDepth = 55.0,
            .monthlyBaseTemps = {11.0, 11.5, 13.0, 15.5, 20.0, 24.5, 27.0, 28.0, 25.5, 22.0, 18.0, 13.5}
        }},
    {"rivio", {
            .name = "Ρίβιο",
            .lat = 38.74,
            .lon = 21.18,
            .maxDepth = 40.0,
            .monthlyBaseTemps = {10.0, 10.5, 12.5, 16.0, 21.0, 25.0, 28.0, 28.5, 25.0, 21.0, 16.0, 12.0}
        }},
    {"ozeros", {
            .name = "Οζερός",
            .lat = 38.65,
            .lon = 21.23,
            .maxDepth = 10.0,
            .monthlyBaseTemps = {8.0, 9.0, 13.0, 17.5, 23.0, 27.0, 29.5, 30.0, 26.0, 20.0, 14.0, 9.5}
        }},
{"voulkaria", {
        .name = "Βουλκαρία",
        .lat = 38.86,
        .lon = 20.83,
        .maxDepth = 2.9,
        .monthlyBaseTemps = {8.0, 9.5, 13.5, 18.0, 24.0, 28.0, 30.0, 30.5, 26.0, 20.5, 14.5, 9.5}
}}
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

    // Αναζήτηση βάσει του String Key
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

    // --- ΒΗΜΑ 1: Ημερήσια Θερμοκρασία Αέρα (Daily Air Average) ---
    double currentTemp = weatherData.contains("Temperature") ? weatherData.at("Temperature") : 0.0;
    double tempMax = weatherData.contains("TempMax") ? weatherData.at("TempMax") : currentTemp;
    double tempMin = weatherData.contains("TempMin") ? weatherData.at("TempMin") : currentTemp;

    double dailyAirTemp = (tempMax + tempMin) / 2.0;

    // --- ΝΕΟ: ΥΠΟΛΟΓΙΣΜΟΣ ΠΡΑΓΜΑΤΙΚΟΥ ΝΕΡΟΥ (Weighted Blending 50/50) ---
    int currentMonth = QDate::currentDate().month();

    // Ιστορικοί μέσοι όροι νερού (Ιαν - Δεκ) για λίμνες (Base Temperature)
    int monthIndex = std::clamp(currentMonth - 1, 0, 11);
    double monthlyWaterTemp = m_currentLake.monthlyBaseTemps[monthIndex];

    // Μίξη 50% Μήνας / 50% Σημερινός Αέρας
    double monthWeight = 0.80;
    double dailyWeight = 1.0 - monthWeight; // Το υπόλοιπο 50%

    double finalWaterTemp = (monthlyWaterTemp * monthWeight) + (dailyAirTemp * dailyWeight);

    // Το dailyTemp (που χρησιμοποιεί όλο το σύστημα κάτω) πλέον είναι η ΜΙΚΤΗ θερμοκρασία!
    double dailyTemp = finalWaterTemp;

    // --- ΒΗΜΑ 2: Πολλαπλασιαστής Φωτός (Daylight Multiplier) ---
    double sunrise = weatherData.contains("Sunrise") ? weatherData.at("Sunrise") : 6.0;
    double sunset = weatherData.contains("Sunset") ? weatherData.at("Sunset") : 18.0;

    double daylightHours = sunset - sunrise;
    if (daylightHours < 0.0) daylightHours += 24.0; // Ασφάλεια για wrap-around τα μεσάνυχτα

    // 12 ώρες μέρα = 1.0 (Ουδέτερο). Κάθε έξτρα ώρα δίνει +5% bonus, κάθε μείον ώρα δίνει -5% penalty.
    double daylightModifier = 1.0 + ((daylightHours - 12.0) * 0.05);
    // Το κρατάμε αυστηρά μεταξύ 0.8x (Βαρύς Χειμώνας) και 1.2x (Κατακαλόκαιρο)
    daylightModifier = std::clamp(daylightModifier, 0.8, 1.2);

    // Φτιάχνουμε ένα αντίγραφο των δεδομένων για να "ταΐσουμε" το ψάρι τη μέση ημερήσια εικόνα
    auto dailyWeatherData = weatherData;
    dailyWeatherData["Temperature"] = dailyTemp;

    // 1. ΥΠΟΛΟΓΙΣΜΟΣ ΕΠΙΦΑΝΕΙΑΣ (Επιλίμνιο)
    double surfaceTemp = dailyTemp;

    ScoreDetails surfaceDetails = targetFish->calculateScore(dailyWeatherData);

    // Εφαρμόζουμε τον Πολλαπλασιαστή και σιγουρεύουμε ότι δεν περνάει το 100% (1.0)
    double finalSurfaceScore = std::clamp(surfaceDetails.totalScore * daylightModifier, 0.0, 1.0);
    const double surfacePct = finalSurfaceScore * 100.0;

    // 2. ΡΥΘΜΙΣΕΙΣ ΛΙΜΝΗΣ & ΥΠΟΛΟΓΙΣΜΟΣ ΘΕΡΜΟΚΛΙΝΑΣ
    double maxDepth = m_currentLake.maxDepth;

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

            auto testWeatherData = dailyWeatherData; // Παίρνουμε το map με τα daily data
            testWeatherData["Temperature"] = tempAtDepth; // Αλλάζουμε μόνο τη θερμοκρασία βάθους

            double testScore = targetFish->calculateScore(testWeatherData).totalScore;

            // Εφαρμόζουμε τον Πολλαπλασιαστή Φωτός και στο βάθος!
            testScore = std::clamp(testScore * daylightModifier, 0.0, 1.0);

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
    weatherStats["surfaceTemp"] = surfaceTemp; // Δείχνει πλέον τη ΜΕΣΗ ΜΙΚΤΗ θερμοκρασία!
    weatherStats["bestDepth"] = bestDepth;
    weatherStats["bestTemp"] = bestTemp;
    weatherStats["airTemp"] = dailyAirTemp; // Έστειλα τον μέσο όρο αέρα για να φαίνεται σωστά στο UI!
    weatherStats["beaufort"] = getBeaufort(windKmh);
    weatherStats["windKmh"] = windKmh;
    weatherStats["compassDir"] = getCompassDirection(m_windDegrees);

    weatherStats["rain"] = weatherData.contains("Precipitation") ? weatherData.at("Precipitation") : 0.0;
    weatherStats["pressure"] = weatherData.contains("Pressure") ? weatherData.at("Pressure") : 0.0;

    const auto& scores = surfaceDetails.parameterScores;
    weatherStats["scoreTemp"] = scores.count("Temperature") ? scores.at("Temperature") : 0.0;
    weatherStats["scorePressure"] = scores.count("Pressure") ? scores.at("Pressure") : 0.0;
    weatherStats["scoreWindDir"] = scores.count("WindDirection") ? scores.at("WindDirection") : 0.0;
    weatherStats["scoreRain"] = scores.count("Precipitation") ? scores.at("Precipitation") : 0.0;

    emit calculationFinished(surfacePct, bestThermoPct, bestDepth, weatherStats);
}

void EngineController::onWeatherError(const std::string& errorMsg) {
    emit calculationError(QString::fromStdString(errorMsg));
};