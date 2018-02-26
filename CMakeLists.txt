cmake_minimum_required(VERSION 2.8)

project(my2048)

add_executable(${PROJECT_NAME} "main.c")
find_package(Curses REQUIRED)
target_link_libraries(${PROJECT_NAME} ${CURSES_LIBRARIES})


