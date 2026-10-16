cmake_minimum_required(VERSION 3.19)

project(TelegramClone LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(TelegramClone
    WIN32 MACOSX_BUNDLE
    src/main.cpp

    src/ui/MainWindow.h src/ui/MainWindow.cpp
    src/ui/MessageDelegate.h src/ui/MessageDelegate.cpp

    src/core/ChatModel.h src/core/ChatModel.cpp
    src/core/MessageModel.h src/core/MessageModel.cpp

    .clang-format
)

target_include_directories(TelegramClone PRIVATE src)

target_link_libraries(TelegramClone
    PRIVATE
    Qt6::Widgets
)