cmake_minimum_required(VERSION 3.16)
project(FishingApp VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Η ΛΥΣΗ: Ενεργοποιούμε τους αυτόματους μεταφραστές του Qt (MOC, RCC, UIC) ---
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# --- Η διαδρομή για το Qt 6.10.2 ---
set(CMAKE_PREFIX_PATH "C:/Qt/6.10.2/mingw_64")

find_package(Qt6 REQUIRED COMPONENTS Core Network Gui Qml Quick QuickControls2)

# --- Διορθώνει το Warning (QTP0001) για το πώς διαβάζει τα αρχεία το Qt 6.10 ---
qt_policy(SET QTP0001 NEW)

# ---------------------------------------------------------------------------

qt_add_executable(FishingApp
        # --- Source Files (.cpp) ---
        main.cpp
        src/Species.cpp
        src/WeatherUtils.cpp
        src/WeatherService.cpp
        src/EngineController.cpp

        # --- Header Files (.hpp) ---
        # ΠΡΟΣΘΗΚΗ: Βάζουμε τα headers για να τα δει ο μεταφραστής (MOC)
        include/FishingEngine/Species.hpp
        include/FishingEngine/WeatherUtils.hpp
        include/FishingEngine/WeatherService.hpp
        include/FishingEngine/EngineController.hpp
)

qt_add_qml_module(FishingApp
        URI FishingEngine
        VERSION 1.0
        QML_FILES
        Main.qml
)

target_link_libraries(FishingApp PRIVATE
        Qt6::Core
        Qt6::Network
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
)