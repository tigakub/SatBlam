cmake_minimum_required(VERSION 3.10)

project(SatBlam)

set(SOURCES
    main.cpp
    App.hpp App.cpp
    Critical.hpp
    Message.hpp Message.cpp
    SmartPtr.hpp
    UDP.hpp UDP.cpp
    )

add_executable(SatBlam ${SOURCES})

target_link_libraries(SatBlam PRIVATE pthread)