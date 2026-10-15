cmake_minimum_required(VERSION 3.19)

project(TelegramClone LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(TelegramClone
    WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/mainwindow.h src/mainwindow.cpp
    .clang-format

)

target_link_libraries(TelegramClone
    PRIVATE
    Qt6::Widgets
)