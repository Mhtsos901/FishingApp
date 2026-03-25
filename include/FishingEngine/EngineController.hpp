#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QtQml/qqml.h>
#include "WeatherService.hpp"
#include "Species.hpp"

struct LakeData {
    QString name;
    double lat;
    double lon;
    double maxDepth;
    std::vector<double> monthlyBaseTemps;
};

class EngineController : public QObject {
    Q_OBJECT

    QML_ELEMENT
    // --- ΝΕΟ: Αυτό επιτρέπει στο QML να διαβάζει τη μεταβλητή m_windDegrees ---
    Q_PROPERTY(double windDegrees READ windDegrees NOTIFY windDegreesChanged)

public:
    explicit EngineController(QObject *parent = nullptr);

    Q_INVOKABLE void calculateCatchProbability(const QString& locationKey, const QString& fishKey);

    double windDegrees() const { return m_windDegrees; }

    signals:
        // Στέλνουμε: % Επιφάνειας, % Θερμοκλίνας, Ιδανικό Βάθος, και το Κείμενο
        void calculationFinished(double surfacePct, double thermoPct, double bestDepth, const QVariantMap& stats);

        void calculationError(const QString& errorMessage);
        void windDegreesChanged();

private slots:
    void onWeatherReady(const std::unordered_map<std::string, double>& weatherData);
    void onWeatherError(const std::string& errorMsg);

private:
    WeatherService* m_weatherService;

    QString m_currentFishKey;
    LakeData m_currentLake;

    double m_windDegrees;
};