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

    // Πρόσθεσε αυτό στο WeatherUtils.cpp
    // (Μην ξεχάσεις να βάλεις τη δήλωσή της και στο WeatherUtils.hpp!)

    double calculateThermoclineDepth(int currentMonth, double maxLakeDepth, double windSpeedKmh) {
        double seasonMultiplier = 0.0;

        // 1. Υπολογισμός Εποχικότητας
        if (currentMonth == 12 || currentMonth == 1 || currentMonth == 2) {
            // Χειμώνας: Αναστροφή λίμνης (Turnover), το νερό είναι ίδιο παντού
            return 0.0;
        }
        else if (currentMonth >= 3 && currentMonth <= 5) {
            // Άνοιξη: Η θερμοκλίνα σχηματίζεται ψηλά
            seasonMultiplier = 0.15;
        }
        else if (currentMonth >= 6 && currentMonth <= 8) {
            // Καλοκαίρι: Ισχυρή διαστρωμάτωση (Stratification)
            seasonMultiplier = 0.25;
        }
        else if (currentMonth >= 9 && currentMonth <= 11) {
            // Φθινόπωρο: Το κρύο νερό βυθίζεται, η θερμοκλίνα πάει βαθιά
            seasonMultiplier = 0.35;
        }

        // 2. Βασικός υπολογισμός βάθους
        double baseThermocline = maxLakeDepth * seasonMultiplier;

        // 3. Επίδραση του αέρα (Wind Mixing)
        // Κάθε 10 km/h αέρα σπρώχνουν τη θερμοκλίνα 1 μέτρο πιο κάτω
        double windEffect = windSpeedKmh / 10.0;

        // 4. Τελικό βάθος
        double finalDepth = baseThermocline + windEffect;

        // Ασφάλεια (Clamp): Η θερμοκλίνα δεν μπορεί να είναι πιο βαθιά από την ίδια τη λίμνη!
        if (finalDepth > maxLakeDepth) {
            finalDepth = maxLakeDepth;
        }

        return finalDepth;
    }

    double calculateTempAtDepth(double surfaceTemp, double z_th, double targetDepth) {
        // 1. Βρισκόμαστε ΠΑΝΩ από τη θερμοκλίνα (Επιλίμνιο)
        // Η θερμοκρασία πέφτει ελάχιστα (0.2°C / μέτρο)
        if (targetDepth <= z_th) {
            return surfaceTemp - (0.2 * targetDepth);
        }

        // 2. Βρισκόμαστε ΜΕΣΑ ή ΚΑΤΩ από τη θερμοκλίνα (Μεταλίμνιο / Υπολίμνιο)
        // Υπολογίζουμε πρώτα τη θερμοκρασία ακριβώς στο σημείο έναρξης (z_th)
        double tempAtZth = surfaceTemp - (0.2 * z_th);

        // Μετά, η θερμοκρασία κάνει "βουτιά" (1.5°C / μέτρο) για το υπόλοιπο βάθος
        double finalTemp = tempAtZth - (1.5 * (targetDepth - z_th));

        // Ασφάλεια: Το νερό στον πάτο μιας λίμνης δεν πέφτει ποτέ κάτω από τους 4°C
        if (finalTemp < 4.0) {
            return 4.0;
        }

        return finalTemp;
    }
}
