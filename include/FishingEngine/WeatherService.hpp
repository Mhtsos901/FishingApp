#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <unordered_map>
#include <string>

class WeatherService : public QObject {
    Q_OBJECT

public:
    explicit WeatherService(QObject *parent = nullptr);

    void getLiveWeather(double latitude, double longitude);

    signals:
        void weatherDataReady(const std::unordered_map<std::string, double>& weatherData);

    void errorOccurred(const std::string& errorMessage);

private slots:
    void onNetworkReply(QNetworkReply* reply);

private:
    QNetworkAccessManager* manager;
};