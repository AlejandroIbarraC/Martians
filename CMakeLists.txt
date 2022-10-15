cmake_minimum_required(VERSION 3.22)
project(ui_martians C)

set(CMAKE_C_STANDARD 23)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

add_executable(ui_martians src/main.c src/codigo/Martian.h src/codigo/Martian_node.c src/codigo/Martian_node.h)
target_link_libraries(ui_martians PRIVATE Threads::Threads ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${GTK3_LIBRARIES} )