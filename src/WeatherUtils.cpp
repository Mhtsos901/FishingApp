#include "../include/FishingEngine/WeatherUtils.hpp"
#include <string>
namespace WeatherUtils {

    double dynamicTemp (double minT, double maxT, double photoperiod) {
        const double nightHours = 24 - photoperiod;
        const double waterTemp = ((maxT * photoperiod) + (minT * nightHours )) / 24.0;

        return waterTemp;
    }

    double highTimeZone (const std::string& isoDatetime) {
        size_t tPos = isoDatetime.find('T');
        if (tPos == std::string::npos) return 0.0;

        std::string timePart = isoDatetime.substr(tPos + 1); // "06:23"
        int hours   = std::stoi(timePart.substr(0, 2));
        int minutes = std::stoi(timePart.substr(3, 2));

        return hours + (minutes / 60.0);
    }


    double calculateThermoclineDepth(int currentMonth, double maxLakeDepth, double windSpeedKmh) {
        double seasonMultiplier = 0.0;

        // seasons calculation
        if (currentMonth == 12 || currentMonth == 1 || currentMonth == 2) {
            return 0.0;
        }
        else if (currentMonth >= 3 && currentMonth <= 5) {
            seasonMultiplier = 0.15;
        }
        else if (currentMonth >= 6 && currentMonth <= 8) {
            seasonMultiplier = 0.25;
        }
        else if (currentMonth >= 9 && currentMonth <= 11) {
            seasonMultiplier = 0.35;
        }

        double baseThermocline = maxLakeDepth * seasonMultiplier;

        // wind mixing
        // every 10 km/h wind pushes the thermocline 1 meter down
        double windEffect = windSpeedKmh / 10.0;

        // fnal depth
        double finalDepth = baseThermocline + windEffect;

        if (finalDepth > maxLakeDepth) {
            finalDepth = maxLakeDepth;
        }

        return finalDepth;
    }

    double calculateTempAtDepth(double surfaceTemp, double z_th, double targetDepth) {


        if (z_th <= 0.0) {
            return surfaceTemp;
        }

        if (targetDepth <= z_th) {
            return surfaceTemp - (0.2 * targetDepth);
        }

        double tempAtZth = surfaceTemp - (0.2 * z_th);

        // 1.5 degrees avery meter you going down
        double finalTemp = tempAtZth - (1.5 * (targetDepth - z_th));

        // safe measures water temp on the bottom doest drop bellow 4 degrees
        return std::max(finalTemp, 4.0);
    }
}
