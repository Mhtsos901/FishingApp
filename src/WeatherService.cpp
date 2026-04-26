#include "../include/FishingEngine/WeatherService.hpp"
#include "../include/FishingEngine/WeatherUtils.hpp"
#include <QNetworkRequest>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <iostream>

WeatherService::WeatherService(QObject *parent) : QObject(parent) {
    manager = new QNetworkAccessManager(this);

    connect(manager, &QNetworkAccessManager::finished, this, &WeatherService::onNetworkReply);
}

void WeatherService::getLiveWeather(double latitude, double longitude) {
    QString urlStr = QString("https://api.open-meteo.com/v1/forecast?latitude=%1&longitude=%2"
                             "&current=surface_pressure,wind_speed_10m,wind_direction_10m,precipitation,temperature_2m"
                             "&daily=temperature_2m_max,temperature_2m_min,daylight_duration,precipitation_sum,wind_direction_10m_dominant,sunset,sunrise"
                             "&timezone=auto").arg(latitude).arg(longitude);

    QNetworkRequest request((QUrl(urlStr)));

    manager->get(request);
}

void WeatherService::onNetworkReply(QNetworkReply* reply) {
    reply->deleteLater();

    // internet access
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred("Network Error: " + reply->errorString().toStdString());
        return;
    }

    // safe decoding JSON Safe Parsing
    QJsonParseError parseError;
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);

    if (parseError.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
        emit errorOccurred("API Error: Το αρχείο JSON είναι κατεστραμμένο.");
        return;
    }

    QJsonObject data = jsonDoc.object();

    if (!data.contains("current") || !data.contains("daily")) {
        emit errorOccurred("API Error: Λείπουν τα δεδομένα 'current' ή 'daily'.");
        return;
    }

    QJsonObject current = data["current"].toObject();
    QJsonObject daily = data["daily"].toObject();

    QJsonArray tmaxArr = daily["temperature_2m_max"].toArray();
    QJsonArray tminArr = daily["temperature_2m_min"].toArray();
    QJsonArray daylightArr = daily["daylight_duration"].toArray();
    QJsonArray windDirArr = daily["wind_direction_10m_dominant"].toArray();
    QJsonArray sunsetArr = daily["sunset"].toArray();
    QJsonArray sunriseArr = daily["sunrise"].toArray();

    QJsonArray rainSumArr = daily["precipitation_sum"].toArray();

    if (tmaxArr.isEmpty() || tminArr.isEmpty() || daylightArr.isEmpty() || windDirArr.isEmpty() || rainSumArr.isEmpty()) {
        emit errorOccurred("API Error: Ελλιπή δεδομένα (Empty Arrays) από το Open-Meteo.");
        return;
    }

    std::unordered_map<std::string, double> weatherData;

    weatherData["Pressure"] = current["surface_pressure"].toDouble();
    weatherData["WindSpeed"] = current["wind_speed_10m"].toDouble();
    weatherData["AirTemperature"] = current["temperature_2m"].toDouble();

    weatherData["WindDirection"] = windDirArr[0].toDouble();

    weatherData["Precipitation"] = rainSumArr[0].toDouble();

    double tmax = tmaxArr[0].toDouble();
    double tmin = tminArr[0].toDouble();
    double daylightSeconds = daylightArr[0].toDouble();
    double photoperiod = daylightSeconds / 3600.0;

    weatherData["Temperature"] = WeatherUtils::dynamicTemp(tmin, tmax, photoperiod);

    if (!sunsetArr.isEmpty() && !sunriseArr.isEmpty()) {
        weatherData["Sunset"] = WeatherUtils::highTimeZone(sunsetArr[0].toString().toStdString());
        weatherData["Sunrise"] = WeatherUtils::highTimeZone(sunriseArr[0].toString().toStdString());
    }

    weatherData["TimeOfDay"] = WeatherUtils::highTimeZone(current["time"].toString().toStdString());

    emit weatherDataReady(weatherData);
}