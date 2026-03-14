#pragma once
#include <QObject>
#include <QString>
#include "WeatherService.hpp"
#include "Species.hpp"

class EngineController : public QObject {
    Q_OBJECT
    // --- ΝΕΟ: Αυτό επιτρέπει στο QML να διαβάζει τη μεταβλητή m_windDegrees ---
    Q_PROPERTY(double windDegrees READ windDegrees NOTIFY windDegreesChanged)

public:
    explicit EngineController(QObject *parent = nullptr);

    Q_INVOKABLE void calculateCatchProbability(int locationId, int fishId);

    // --- ΝΕΟ: Getter συνάρτηση για το Property ---
    double windDegrees() const { return m_windDegrees; }

    signals:
        void calculationFinished(double percentage, const QString& debugInfo);
    void calculationError(const QString& errorMessage);
    // --- ΝΕΟ: Σήμα που λέει στο QML "ο αέρας άλλαξε, ξαναζωγράφισε το βέλος" ---
    void windDegreesChanged();

private slots:
    void onWeatherReady(const std::unordered_map<std::string, double>& weatherData);
    void onWeatherError(const std::string& errorMsg);

private:
    WeatherService* m_weatherService;
    int m_currentFishId;
    double m_windDegrees; // <--- ΝΕΟ: Αποθήκευση των μοιρών εδώ
};