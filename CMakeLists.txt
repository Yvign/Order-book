cmake_minimum_required(VERSION 3.14)
project(obook CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(FTXUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_DOCS     OFF CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
    GIT_TAG        v5.0.0
)
FetchContent_MakeAvailable(ftxui)

add_executable(obook
    main.cpp
    OrderBook.cpp
)

target_link_libraries(obook
    PRIVATE
    ftxui::screen
    ftxui::dom
    ftxui::component
)
