#pragma once

#include <string>
#include <unordered_map>

class WeatherService {
public:
    static std::unordered_map<std::string, double> getLiveWeather(double latitude, double longitude);
};