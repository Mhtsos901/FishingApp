cmake_minimum_required(VERSION 4.1)
project(FishingApp)

set(CMAKE_CXX_STANDARD 20)

# --- Η ΛΥΣΗ ΓΙΑ ΤΑ DLLs (Στατική Σύνδεση) ---

set(BUILD_SHARED_LIBS off)
set(CMAKE_EXE_LINKER_FLAGS "-static")

# --- 1. Ενεργοποιούμε το εργαλείο λήψης ---
include(FetchContent)

# --- 2. Κατεβάζουμε το JSON ---
FetchContent_Declare(json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

# --- 3. Κατεβάζουμε την CPR (Internet) ---
FetchContent_Declare(cpr
        GIT_REPOSITORY https://github.com/libcpr/cpr.git
        GIT_TAG 1.10.5
)
FetchContent_MakeAvailable(cpr)

# --- 4. Δηλώνουμε ΟΛΑ τα αρχεία του project ---
add_executable(FishingApp
        main.cpp
        src/Species.cpp
        src/WeatherUtils.cpp
        src/WeatherService.cpp
)

# --- 5. Συνδέουμε τις βιβλιοθήκες ---
target_link_libraries(FishingApp PRIVATE cpr::cpr nlohmann_json::nlohmann_json)