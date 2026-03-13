#include <iostream>
#include <unordered_map>
#include <string>

#include "include/FishingEngine/WeatherService.hpp"
#include "include/FishingEngine/Species.hpp"

int main() {
    std::cout << "======================================" << std::endl;
    std::cout << "  Fishing Engine: Live Data v2.0      " << std::endl;
    std::cout << "======================================" << std::endl;
    std::cout << "Ginetai lipsi live kairou gia Agrinio...\n" << std::endl;

    // 1. Ζητάμε τα ζωντανά δεδομένα (Γαβαλου)
    std::unordered_map<std::string, double> todayWeather = WeatherService::getLiveWeather(38.52, 21.52);

    // 2. Έλεγχος Ασφαλείας (Safety Check): Αν το ίντερνετ κόπηκε ή το API απέτυχε
    if (todayWeather.empty()) {
        std::cout << "-> Sfalma: Den mporesame na feroume ta dedomena kairou." << std::endl;
        return 1; // Τερματίζουμε το πρόγραμμα με κωδικό σφάλματος 1
    }

    std::cout << "choose 1 for grivadi or 2 for petalouda: ";
    int x{};
    std::cin >> x;
    if (x == 1) {
        Species targetFish("grivadi");
        const double rawScore = targetFish.calculateScore(todayWeather);
        const double finalPercentage = rawScore * 100.0;
        std::cout << "Stoxos: Grivadi" << std::endl;
        std::cout << "Teliko pososto epityxias: " << finalPercentage << "%" << std::endl;
    }
    else if (x == 2) {
        Species targetFish("petalouda");
        const double rawScore = targetFish.calculateScore(todayWeather);
        const double finalPercentage = rawScore * 100.0;
        std::cout << "Stoxos: Petalouda" << std::endl;
        std::cout << "Teliko pososto epityxias: " << finalPercentage << "%" << std::endl;
    }

    // 4. Εκτύπωση Αποτελεσμάτων
    std::cout << "--------------------------------------" << std::endl;
    std::cout << "[Live Data Debug]" << std::endl;
    std::cout << "Water temperature : " << todayWeather["Temperature"] << " Celcius" << std::endl;
    std::cout << "Wind Speed        : " << todayWeather["WindSpeed"] << " km/h" << std::endl;
    std::cout << "Wind Direction    : " << todayWeather["WindDirection"] << " / 100 (Score)" << std::endl;
    std::cout << "======================================" << std::endl;

    return 0;
}