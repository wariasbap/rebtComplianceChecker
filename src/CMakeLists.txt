# src/CMakeLists.txt

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
)

add_executable(rebt_checker ${SOURCES} ${HEADERS})

target_link_libraries(rebt_checker
    PRIVATE
        nlohmann_json::nlohmann_json
)

target_include_directories(rebt_checker
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

