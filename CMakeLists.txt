cmake_minimum_required(VERSION 3.16)

project(EnetLearn LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED 1)

add_executable(client WIN32 
    "src/client_main.cpp" 
    "src/mpr_window.cpp" 
    "src/connect_dialog.cpp" 
    "src/connect_dialog.rc"
    )

target_link_libraries(client PRIVATE "${PROJECT_SOURCE_DIR}/enet.lib" winmm gdiplus ws2_32 user32 gdi32)
target_include_directories(client PRIVATE "${PROJECT_SOURCE_DIR}/")

add_executable(server "src/server_main.cpp")
target_link_libraries(server PRIVATE "${PROJECT_SOURCE_DIR}/enet.lib" winmm ws2_32 iphlpapi)
target_include_directories(server PRIVATE "${PROJECT_SOURCE_DIR}/")
