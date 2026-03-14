#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <unordered_map>
#include <string>

// Το Q_OBJECT macro είναι υποχρεωτικό για να χρησιμοποιούμε Signals & Slots
class WeatherService : public QObject {
    Q_OBJECT

public:
    // O constructor παίρνει πάντα ένα parent QObject για αυτόματη διαχείριση μνήμης (Memory Management)
    explicit WeatherService(QObject *parent = nullptr);

    // Ξεκινάει το request και επιστρέφει αμέσως (Non-blocking)
    void getLiveWeather(double latitude, double longitude);

    signals:
        // Αυτό το σήμα (Signal) εκπέμπεται ΟΤΑΝ τα δεδομένα έχουν κατέβει και αναλυθεί
        void weatherDataReady(const std::unordered_map<std::string, double>& weatherData);

    // Σήμα σε περίπτωση που κοπεί το ίντερνετ
    void errorOccurred(const std::string& errorMessage);

private slots:
    // Αυτή η συνάρτηση (Slot) καλείται αυτόματα όταν το API απαντήσει
    void onNetworkReply(QNetworkReply* reply);

private:
    QNetworkAccessManager* manager;
};