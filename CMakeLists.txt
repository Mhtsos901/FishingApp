cmake_minimum_required(VERSION 3.16)
project(FishingApp VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Ενεργοποιούμε τους αυτόματους μεταφραστές του Qt (MOC, RCC, UIC) ---
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# --- Η διαδρομή για το Qt 6.10.2 ---
set(CMAKE_PREFIX_PATH "C:/Qt/6.10.2/mingw_64")

find_package(Qt6 REQUIRED COMPONENTS Core Network Gui Qml Quick QuickControls2)

# --- Διορθώνει το Warning (QTP0001) για το πώς διαβάζει τα αρχεία το Qt 6.10 ---
qt_policy(SET QTP0001 NEW)

# ---------------------------------------------------------------------------

# 1. THE EXECUTABLE: Το εκτελέσιμο πλέον περιέχει μόνο το entry point.
qt_add_executable(FishingApp
        main.cpp
)

set_target_properties(FishingApp PROPERTIES
   QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")

# 2. THE QML MODULE: Εδώ "πακετάρουμε" όλη την επιχειρησιακή λογική (Domain Logic) και το UI.
qt_add_qml_module(FishingApp
        URI FishingEngine
        VERSION 1.0

        QML_FILES
        Main.qml

        # --- Δηλώνουμε τα C++ αρχεία για να μπορεί να τα διαβάσει το εργαλείο QML Registration ---
        SOURCES
        src/Species.cpp
        src/WeatherUtils.cpp
        src/WeatherService.cpp
        src/EngineController.cpp

        include/FishingEngine/Species.hpp
        include/FishingEngine/WeatherUtils.hpp
        include/FishingEngine/WeatherService.hpp
        include/FishingEngine/EngineController.hpp
)

# 3. INCLUDE DIRECTORIES: Λέμε στον compiler πού να ψάχνει για τα Header Files.
# Αυτό είναι απαραίτητο για να μη "σκάει" το qmltyperegistrations.cpp!
target_include_directories(FishingApp PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/FishingEngine
)

# 4. LINKING: Συνδέουμε τις επίσημες βιβλιοθήκες του Qt.
target_link_libraries(FishingApp PRIVATE
        Qt6::Core
        Qt6::Network
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
)

# --- Android OpenSSL 3 (The REAL Final Fix) ---
if(ANDROID)
    # 1. Συνδέουμε τον φάκελο android που δημιουργήσαμε
    set_target_properties(FishingApp PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
    )

    # 2. Προσθέτουμε τις βιβλιοθήκες OpenSSL 3 στο πακέτο της εφαρμογής
    set_property(TARGET FishingApp APPEND PROPERTY QT_ANDROID_EXTRA_LIBS
        "C:/Android/android_openssl-master/ssl_3/arm64-v8a/libcrypto_3.so"
        "C:/Android/android_openssl-master/ssl_3/arm64-v8a/libssl_3.so"
    )
endif()