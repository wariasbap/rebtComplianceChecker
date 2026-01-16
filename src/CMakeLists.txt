# src/CMakeLists.txt

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(SOURCES
    main.cpp
)

set(HEADERS
    Geometry.hpp
    Routing.hpp
    Electrical.hpp
    JsonAdapters.hpp
    SvgHelper.hpp
    Visualise.hpp
    InstallationView.hpp
)

add_executable(rebt_checker ${SOURCES} ${HEADERS})

find_package(Qt6 REQUIRED COMPONENTS Widgets)

target_link_libraries(rebt_checker
    PRIVATE
        Qt6::Widgets
        nlohmann_json::nlohmann_json
)

target_include_directories(rebt_checker
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

