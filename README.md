# FishingApp 🎣

A real-time fishing conditions engine for Greek lakes, built in C++20 and Qt6/QML.

I built this as a personal project to combine things I enjoy — fishing, C++, and working with real data.
It fetches live weather from the Open-Meteo API and runs it through a custom scoring model to estimate
how good the conditions are for a specific fish species at a specific lake.

---

## How it works

The core idea is a weighted **Asymmetrical Gaussian Falloff** algorithm. Each fish species has a set of
biological parameters (ideal temperature, pressure tolerance, preferred wind direction, etc.).
The engine takes live weather data and calculates how far each condition is from the ideal,
then combines them into a single Feeding Activity Score (0–100%).

The engine also simulates **thermocline depth** using LERP-based monthly water temperatures and
seasonal stratification physics — so it can calculate depth-specific scores, not just surface conditions.

## Architecture

| Component | Responsibility |
|---|---|
| `WeatherService` | Async REST fetch from Open-Meteo via Qt Network (non-blocking) |
| `Species` | Per-species biological parameters + Gaussian scoring logic |
| `WeatherUtils` | Thermocline physics, LERP interpolation, time parsing |
| `EngineController` | Orchestrates the pipeline, exposes results to QML via Qt signals |

## Tech Stack

`C++20` · `Qt6 / QML` · `CMake` · `Open-Meteo API` · `Android`

## Build

```bash
git clone https://github.com/Mhtsos901/FishingApp.git
cd FishingApp
cmake -B build -DCMAKE_PREFIX_PATH="path/to/Qt/6.x.x/mingw_64"
cmake --build build
```

Requires Qt 6.5+ and CMake 3.16+. Open-Meteo is free and needs no API key.

## Current Species

- **Common Carp** (Γριβάδι) — fully parameterised
- **Petalouda** — in progress

## Notes

The QML frontend was AI-assisted — my focus was on the C++ engine and scoring logic.
The lakes covered are in the Agrinio region of Greece: Trichonida, Ozeros, Rivio, Voulkaria.
