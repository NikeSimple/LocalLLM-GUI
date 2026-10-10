cmake_minimum_required(VERSION 3.16)
project(LocalLLM-GUI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Widgets
    Network
    Sql
    Svg
)

set(SOURCES
    main.cpp
    gui/main_window.cpp
    gui/chat_widget.cpp
    gui/dialog_list_widget.cpp
    gui/settings_panel.cpp
    gui/settings_overlay.cpp
    gui/toggle_switch.cpp
    gui/theme_manager.cpp
    core/api_client.cpp
    core/file_handler.cpp
    db/database_manager.cpp
    db/dialog_repository.cpp
    db/message_repository.cpp
    db/prompt_repository.cpp
    config/settings_manager.cpp
    resources/resources.qrc
    resources/app.rc
)

add_executable(LocalLLM-GUI ${SOURCES})

target_include_directories(LocalLLM-GUI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(LocalLLM-GUI PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
    Qt6::Svg
)

enable_testing()
add_executable(tests tests/test_main.cpp)

find_library(SQLITE3_LIB NAMES sqlite3 PATHS /ucrt64/lib /usr/lib64 /usr/lib)
if(SQLITE3_LIB)
    target_link_libraries(tests PRIVATE ${SQLITE3_LIB})
endif()

add_test(NAME unit_tests COMMAND tests)