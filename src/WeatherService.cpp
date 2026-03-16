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
    // Best Practice: Αποφυγή Memory Leak
    reply->deleteLater();

    // 1. Έλεγχος δικτύου (π.χ. κομμένο ίντερνετ)
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred("Network Error: " + reply->errorString().toStdString());
        return;
    }

    // 2. Ασφαλής αποκωδικοποίηση JSON (Safe Parsing)
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll(), &parseError);

    // Ελέγχουμε αν το κείμενο ήταν όντως έγκυρο JSON
    if (parseError.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
        emit errorOccurred("API Error: Το αρχείο JSON είναι κατεστραμμένο.");
        return;
    }

    QJsonObject data = jsonDoc.object();

    // 3. DEFENSIVE CHECK: Υπάρχουν τα βασικά αντικείμενα;
    if (!data.contains("current") || !data.contains("daily")) {
        emit errorOccurred("API Error: Λείπουν τα δεδομένα 'current' ή 'daily'.");
        return;
    }

    QJsonObject current = data["current"].toObject();
    QJsonObject daily = data["daily"].toObject();

    // 4. ΑΣΦΑΛΗΣ ΕΞΑΓΩΓΗ ARRAYS (Δεν εμπιστευόμαστε ότι έχουν μέγεθος > 0)
    QJsonArray tmaxArr = daily["temperature_2m_max"].toArray();
    QJsonArray tminArr = daily["temperature_2m_min"].toArray();
    QJsonArray daylightArr = daily["daylight_duration"].toArray();
    QJsonArray windDirArr = daily["wind_direction_10m_dominant"].toArray();
    QJsonArray sunsetArr = daily["sunset"].toArray();
    QJsonArray sunriseArr = daily["sunrise"].toArray();

    // Αν κάποιο από τα κρίσιμα arrays είναι άδειο, σταματάμε αμέσως!
    if (tmaxArr.isEmpty() || tminArr.isEmpty() || daylightArr.isEmpty() || windDirArr.isEmpty()) {
        emit errorOccurred("API Error: Ελλιπή δεδομένα (Empty Arrays) από το Open-Meteo.");
        return;
    }

    // 5. ΠΛΕΟΝ ΕΙΜΑΣΤΕ ΑΣΦΑΛΕΙΣ ΝΑ ΓΕΜΙΣΟΥΜΕ ΤΟ MAP
    std::unordered_map<std::string, double> weatherData;

    weatherData["Pressure"] = current["surface_pressure"].toDouble();
    weatherData["WindSpeed"] = current["wind_speed_10m"].toDouble();
    weatherData["Precipitation"] = current["precipitation"].toDouble();
    weatherData["AirTemperature"] = current["temperature_2m"].toDouble();

    // ΝΕΟ: Πετάξαμε τη συνάρτηση windDirectionSC. Τώρα περνάμε τις ΚΑΘΑΡΕΣ μοίρες!
    weatherData["WindDirection"] = windDirArr[0].toDouble();

    double tmax = tmaxArr[0].toDouble();
    double tmin = tminArr[0].toDouble();
    double daylightSeconds = daylightArr[0].toDouble();
    double photoperiod = daylightSeconds / 3600.0;

    weatherData["Temperature"] = WeatherUtils::dynamicTemp(tmin, tmax, photoperiod);

    // Εξαγωγή Ώρας (με έλεγχο ασφαλείας για Ανατολή/Δύση)
    if (!sunsetArr.isEmpty() && !sunriseArr.isEmpty()) {
        weatherData["Sunset"] = WeatherUtils::highTimeZone(sunsetArr[0].toString().toStdString());
        weatherData["Sunrise"] = WeatherUtils::highTimeZone(sunriseArr[0].toString().toStdString());
    }

    // ΝΕΟ: Μετονομάσαμε το "TimeZone" σε "TimeOfDay" για να ταιριάζει με τον νέο κανόνα στο Species!
    weatherData["TimeOfDay"] = WeatherUtils::highTimeZone(current["time"].toString().toStdString());

    // 6. ΕΚΠΟΜΠΗ ΣΗΜΑΤΟΣ (Όλα πήγαν τέλεια)
    emit weatherDataReady(weatherData);
}