#include "../include/FishingEngine/WeatherService.hpp"
#include "../include/FishingEngine/WeatherUtils.hpp"
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

std::unordered_map<std::string, double> WeatherService::getLiveWeather(double latitude, double longitude) {
    std::unordered_map<std::string, double> weatherData;

    std::string url = "https://api.open-meteo.com/v1/forecast"
                      "?latitude=" + std::to_string(latitude) +
                      "&longitude=" + std::to_string(longitude) +
                      "&current=surface_pressure,wind_speed_10m,wind_direction_10m"
                      "&daily=temperature_2m_max,temperature_2m_min,daylight_duration"
                      "&timezone=auto";

    // ΠΡΟΣΘΗΚΗ: Κλείνουμε τον αυστηρό έλεγχο πιστοποιητικών (VerifySsl{false})
    cpr::Response r = cpr::Get(cpr::Url{url}, cpr::Timeout{5000});


    // Αν όλα πήγαν καλά (Κωδικός 200)
    if (r.status_code == 200) {
        json data = json::parse(r.text);

        std::cout << "\n[RAW API DATA]" << std::endl;
        std::cout << "Piezh: " << data["current"]["surface_pressure"] << " hPa" << std::endl;
        std::cout << "Aeras (Taxuthta): " << data["current"]["wind_speed_10m"] << " km/h" << std::endl;
        std::cout << "Aeras (Moires): " << data["current"]["wind_direction_10m"] << " degrees" << std::endl;
        std::cout << "----------------\n" << std::endl;

        weatherData["Pressure"] = data["current"]["surface_pressure"];
        weatherData["WindSpeed"] = data["current"]["wind_speed_10m"];

        double tmax = data["daily"]["temperature_2m_max"][0];
        double tmin = data["daily"]["temperature_2m_min"][0];
        double daylightSeconds = data["daily"]["daylight_duration"][0];

        double photoperiod = daylightSeconds / 3600.0;
        weatherData["Photoperiod"] = photoperiod;
        weatherData["Temperature"] = WeatherUtils::dynamicTemp(tmin, tmax, photoperiod);

        weatherData["WindDirection"] = WeatherUtils::windDirectionSC(data["current"]["wind_direction_10m"]);

    } else {
        // Αν αποτύχει, τυπώνουμε τον ΑΚΡΙΒΗ λόγο (Error Logging)
        std::cerr << "\n--- ΣΦΑΛΜΑ ΔΙΚΤΥΟΥ ---" << std::endl;
        std::cerr << "HTTP Code: " << r.status_code << std::endl;
        std::cerr << "Mhnuma cpr: " << r.error.message << std::endl;
        std::cerr << "----------------------\n" << std::endl;

        // Επιστρέφουμε έναν εντελώς άδειο χάρτη για να το καταλάβει η main
        return {};
    }

    return weatherData;
}