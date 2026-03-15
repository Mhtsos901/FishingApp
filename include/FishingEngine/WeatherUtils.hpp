#pragma once

#include <string>

namespace WeatherUtils {
    double dynamicTemp (double minT, double maxT, double photoperiod);
    double windDirectionSC (double degrees);
    double highTimeZone (const std::string& isoDatetime);
    double calculateThermoclineDepth(int currentMonth, double maxLakeDepth, double windSpeedKmh);
    double calculateTempAtDepth(double surfaceTemp, double z_th, double targetDepth);
}
