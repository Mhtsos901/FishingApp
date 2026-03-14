#include "../include/FishingEngine/WeatherUtils.hpp"
#include <string>
namespace WeatherUtils {

    double dynamicTemp (double minT, double maxT, double photoperiod) {
        const double nightHours = 24 - photoperiod;
        const double waterTemp = ((maxT * photoperiod) + (minT * nightHours )) / 24.0;

        return waterTemp;
    }

    double windDirectionSC(double degrees) {
        //north
        if ((degrees >= 337.5 && degrees <= 360) || (degrees >= 0 && degrees < 22.5)) {
            return 20.0;
        }
        //northeast
        else if (degrees >= 22.5 && degrees < 67.5) {
            return 10.0;
        }
        //east
        else if (degrees >= 67.5 && degrees < 112.5) {
            return 25.0;
        }
        //southeast
        else if (degrees >=112.5 && degrees < 157.5) {
            return 75.0;
        }
        //south
        else if (degrees >= 157.5 && degrees < 202.5) {
            return 100.0;
        }
        //southwest
        else if (degrees >= 202.5 && degrees < 247.5) {
            return 80.0;
        }
        //west
        else if (degrees >= 247.5 && degrees < 292.5) {
            return 60.0;
        }
        //northwest
        else if (degrees >= 292.5 && degrees < 337.5) {
            return 30.0;
        }
        return 0.0;
    }

    double highTimeZone (const std::string& isoDatetime) {
        size_t tPos = isoDatetime.find('T');
        if (tPos == std::string::npos) return 0.0;

        std::string timePart = isoDatetime.substr(tPos + 1); // "06:23"
        int hours   = std::stoi(timePart.substr(0, 2));
        int minutes = std::stoi(timePart.substr(3, 2));

        return hours + (minutes / 60.0);
    }
}
