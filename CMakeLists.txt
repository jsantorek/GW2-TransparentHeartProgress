cmake_minimum_required(VERSION 3.5.0...3.31.2)
find_package(Git)
execute_process(
    COMMAND
        ${GIT_EXECUTABLE} describe --tags --dirty --abbrev=0 --match "v*.*.*"
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_DESCRIBE_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(GIT_DESCRIBE_VERSION)
    string(REGEX REPLACE "^v" "" PROJECT_VERSION ${GIT_DESCRIBE_VERSION})
    string(REGEX REPLACE "-dirty" ".1" PROJECT_VERSION ${PROJECT_VERSION})
    message("Git describe based versioning: ${PROJECT_VERSION}")
else()
    string(TIMESTAMP TIMESTAMP_VERSION "%Y;%m;%d;%H%M" UTC)
    set(PROJECT_VERSION "")
    foreach(part IN LISTS TIMESTAMP_VERSION)
        string(REGEX REPLACE "^0+([0-9])" "\\1" part "${part}")
        if(part STREQUAL "")
            list(APPEND PROJECT_VERSION "0")
        else()
            list(APPEND PROJECT_VERSION "${part}")
        endif()
    endforeach()
    string(REPLACE ";" "." PROJECT_VERSION "${PROJECT_VERSION}")
    message("Timestamp based versioning: ${PROJECT_VERSION}")
endif()

get_filename_component(PROJECT_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
string(REGEX REPLACE "^[^-]+-" "" PROJECT_NAME "${PROJECT_NAME}")
project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES C CXX)

find_package(rcgg-nexus-api)
find_package(imgui)
find_package(nlohmann_json)

include(${CMAKE_CURRENT_SOURCE_DIR}/src-re/GW2RE.cmake)
file(GLOB_RECURSE SRCs ${CMAKE_CURRENT_SOURCE_DIR}/src/*.[hc]pp)
add_library(${PROJECT_NAME}
    SHARED
        ${SRCs}
        ${RE_SRCs}
)

target_link_libraries(${PROJECT_NAME}
    rcgg-nexus-api::rcgg-nexus-api
    imgui::imgui
    nlohmann_json::nlohmann_json
    ${RE_LIBs}
)
string(REGEX REPLACE "[^A-Z]" "" PROJECT_ACRONYM "${PROJECT_NAME}")
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        ADDON_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        ADDON_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        ADDON_VERSION_PATCH=${PROJECT_VERSION_PATCH}
        ADDON_VERSION_REVISION=$<IF:$<BOOL:${PROJECT_VERSION_TWEAK}>,${PROJECT_VERSION_TWEAK},0>
        ADDON_NAME="${PROJECT_NAME}"
        ADDON_ACRONYM="${PROJECT_ACRONYM}"
)
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${RE_INCLUDE_DIRS}
)
string(REGEX REPLACE "([a-z0-9])([A-Z])" "\\1_\\2" PROJECT_FILENAME "${PROJECT_NAME}")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_FILENAME})
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION "."
)
message("${PROJECT_NAME} v${PROJECT_VERSION} ${PROJECT_FILENAME}.dll")
