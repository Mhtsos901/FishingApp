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

    // Συνδέουμε (Connect) το σήμα "finished" του manager με το δικό μας Slot
    connect(manager, &QNetworkAccessManager::finished, this, &WeatherService::onNetworkReply);
}

void WeatherService::getLiveWeather(double latitude, double longitude) {
    // Χτίζουμε το URL δυναμικά με το QString (πολύ πιο γρήγορο από το std::string +)
    QString urlStr = QString("https://api.open-meteo.com/v1/forecast?latitude=%1&longitude=%2"
                             "&current=surface_pressure,wind_speed_10m,wind_direction_10m,precipitation,temperature_2m"
                             "&daily=temperature_2m_max,temperature_2m_min,daylight_duration,precipitation_sum,wind_direction_10m_dominant,sunset,sunrise"
                             "&timezone=auto").arg(latitude).arg(longitude);

    QNetworkRequest request((QUrl(urlStr)));

    // Ξεκινάμε το GET request. Το πρόγραμμα ΔΕΝ σταματάει εδώ!
    manager->get(request);
}

void WeatherService::onNetworkReply(QNetworkReply* reply) {
    // Best Practice: Διαγράφουμε το reply object όταν τελειώσει το event loop
    // για να αποφύγουμε διαρροή μνήμης (Memory Leak)
    reply->deleteLater();

    // 1. Έλεγχος Σφαλμάτων (Error Handling)
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(reply->errorString().toStdString());
        return;
    }

    // 2. Διάβασμα Δεδομένων
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

    if (!jsonDoc.isObject()) {
        emit errorOccurred("Invalid JSON format from API");
        return;
    }

    // 3. Εξαγωγή JSON (JSON Parsing - Qt Way)
    QJsonObject data = jsonDoc.object();
    QJsonObject current = data["current"].toObject();
    QJsonObject daily = data["daily"].toObject();

    std::unordered_map<std::string, double> weatherData;

    // Γεμίζουμε το map όπως έκανες πριν, αλλά χρησιμοποιώντας τις μεθόδους του Qt (toDouble, toArray, toString)
    weatherData["Pressure"] = current["surface_pressure"].toDouble();
    weatherData["WindSpeed"] = current["wind_speed_10m"].toDouble();
    weatherData["Precipitation"] = current["precipitation"].toDouble();
    weatherData["AirTemperature"] = current["temperature_2m"].toDouble();
    weatherData["RawWindDirection"] = current["wind_direction_10m"].toDouble();

    double tmax = daily["temperature_2m_max"].toArray()[0].toDouble();
    double tmin = daily["temperature_2m_min"].toArray()[0].toDouble();
    double daylightSeconds = daily["daylight_duration"].toArray()[0].toDouble();

    double photoperiod = daylightSeconds / 3600.0;
    weatherData["Photoperiod"] = photoperiod;
    weatherData["Temperature"] = WeatherUtils::dynamicTemp(tmin, tmax, photoperiod);

    weatherData["WindDirection"] = WeatherUtils::windDirectionSC(daily["wind_direction_10m_dominant"].toArray()[0].toDouble());

    // Μετατροπή των QString σε std::string για να είναι συμβατά με τη δική σου WeatherUtils
    weatherData["Sunset"] = WeatherUtils::highTimeZone(daily["sunset"].toArray()[0].toString().toStdString());
    weatherData["Sunrise"] = WeatherUtils::highTimeZone(daily["sunrise"].toArray()[0].toString().toStdString());
    weatherData["TimeZone"] = WeatherUtils::highTimeZone(current["time"].toString().toStdString());

    // 4. ΕΚΠΟΜΠΗ ΣΗΜΑΤΟΣ (Emit Signal)!
    // Αυτό λέει στο υπόλοιπο πρόγραμμα: "Πάρτε τα δεδομένα, είναι έτοιμα!"
    emit weatherDataReady(weatherData);
}