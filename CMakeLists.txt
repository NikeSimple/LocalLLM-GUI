cmake_minimum_required(VERSION 3.16)
project(LocalLLM-GUI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Sql)

set(SOURCES
    main.cpp
    gui/main_window.cpp
    gui/chat_widget.cpp
    gui/dialog_list_widget.cpp
    gui/settings_panel.cpp
    gui/settings_dialog.cpp
    gui/model_selector_widget.cpp
    gui/connect_dialog.cpp
    gui/toggle_switch.cpp
    gui/theme_manager.cpp
    core/api_client.cpp
    core/conversation.cpp
    core/file_handler.cpp
    core/prompt_manager.cpp
    core/context_builder.cpp
    db/database_manager.cpp
    db/dialog_repository.cpp
    db/message_repository.cpp
    db/prompt_repository.cpp
    config/settings_manager.cpp
    config/logger.cpp
    resources/resources.qrc
)

add_executable(LocalLLM-GUI ${SOURCES})

target_include_directories(LocalLLM-GUI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(LocalLLM-GUI PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
)

enable_testing()
add_executable(tests tests/test_main.cpp)
add_test(NAME unit_tests COMMAND tests)