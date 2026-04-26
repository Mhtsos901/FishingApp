#include "../include/FishingEngine/EngineController.hpp"
#include "../include/FishingEngine/WeatherUtils.hpp"
#include <QString>
#include <QDate>
#include <cmath>
#include <memory>

EngineController::EngineController(QObject *parent) : QObject(parent) {
    m_weatherService = new WeatherService(this);

    connect(m_weatherService, &WeatherService::weatherDataReady, this, &EngineController::onWeatherReady);
    connect(m_weatherService, &WeatherService::errorOccurred, this, &EngineController::onWeatherError);
    m_windDegrees = 0.0;
}


void EngineController::calculateCatchProbability(const QString& locationKey, const QString& fishKey) {
    m_currentFishKey = fishKey;

    // lake data
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


    //
    if (!lakes.contains(locationKey)) {
        emit calculationError("Σφάλμα: Άγνωστη τοποθεσία!");
        return;
    }

    // Saving the lakedata onWeatherReady
    m_currentLake = lakes.value(locationKey);

    m_weatherService->getLiveWeather(m_currentLake.lat, m_currentLake.lon);
}

void EngineController::onWeatherReady(const std::unordered_map<std::string, double>& weatherData) {
    if (weatherData.empty()) {
        emit calculationError("Σφάλμα: Δεν βρέθηκαν δεδομένα καιρού.");
        return;
    }

    std::unique_ptr<Species> targetFish;

    // searching the fish with String Key
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

    // Daily Air Average
    double currentTemp = weatherData.contains("Temperature") ? weatherData.at("Temperature") : 0.0;
    double tempMax = weatherData.contains("TempMax") ? weatherData.at("TempMax") : currentTemp;
    double tempMin = weatherData.contains("TempMin") ? weatherData.at("TempMin") : currentTemp;

    double dailyAirTemp = (tempMax + tempMin) / 2.0;

    // Weighted Blending
    int currentMonth = QDate::currentDate().month();

    // LERP
    QDate today = QDate::currentDate();
    int currentMonthIdx = today.month() - 1; // 0--11
    int nextMonthIdx = (currentMonthIdx + 1) % 12;

    double currentMonthTemp = m_currentLake.monthlyBaseTemps[currentMonthIdx];
    double nextMonthTemp = m_currentLake.monthlyBaseTemps[nextMonthIdx];

    double progress = static_cast<double>(today.day()) / today.daysInMonth();

    // LERP: blended = (1.0 - t) * A + t * B
    double interpolatedWaterTemp = (1.0 - progress) * currentMonthTemp + (progress * nextMonthTemp);

    // 80% historical LERP / 20% air
    double monthWeight = 0.80;
    double dailyWeight = 1.0 - monthWeight;

    double finalWaterTemp = (interpolatedWaterTemp * monthWeight) + (dailyAirTemp * dailyWeight);

    double dailyTemp = finalWaterTemp;

    // Daylight Multiplier
    double sunrise = weatherData.contains("Sunrise") ? weatherData.at("Sunrise") : 6.0;
    double sunset = weatherData.contains("Sunset") ? weatherData.at("Sunset") : 18.0;

    double daylightHours = sunset - sunrise;
    if (daylightHours < 0.0) daylightHours += 24.0;

    // 12 hours day = 1.0 (neutral). every extra hour gives +5% bonus and every less hour gives -5% penalty
    double daylightModifier = 1.0 + ((daylightHours - 12.0) * 0.05);
    // 0.8x heavy winter και 1.2x middle of the summer
    daylightModifier = std::clamp(daylightModifier, 0.8, 1.2);

    auto dailyWeatherData = weatherData;
    dailyWeatherData["Temperature"] = dailyTemp;

    // calculate surface temp
    double surfaceTemp = dailyTemp;

    ScoreDetails surfaceDetails = targetFish->calculateScore(dailyWeatherData);

    double finalSurfaceScore = std::clamp(surfaceDetails.totalScore * daylightModifier, 0.0, 1.0);
    const double surfacePct = finalSurfaceScore * 100.0;

    double maxDepth = m_currentLake.maxDepth;

    double windKmh = weatherData.contains("WindSpeed") ? weatherData.at("WindSpeed") : 0.0;
    double z_th = WeatherUtils::calculateThermoclineDepth(currentMonth, maxDepth, windKmh);

    // Linear Search Algorithm
    double bestThermoPct = 0.0;
    double bestDepth = 0.0;
    double bestTemp = surfaceTemp;

    if (z_th > 0.0 && z_th < maxDepth) {
        double maxScoreFound = -1.0;
        int startDepth = static_cast<int>(std::ceil(z_th));
        int endDepth = static_cast<int>(maxDepth);

        for (int currentDepth = startDepth; currentDepth <= endDepth; ++currentDepth) {
            double tempAtDepth = WeatherUtils::calculateTempAtDepth(surfaceTemp, z_th, currentDepth);

            auto testWeatherData = dailyWeatherData;
            testWeatherData["Temperature"] = tempAtDepth;

            double testScore = targetFish->calculateScore(testWeatherData).totalScore;

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
    weatherStats["airTemp"] = dailyAirTemp;
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