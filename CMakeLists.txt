cmake_minimum_required(VERSION 3.10)

project(SatBlam)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Dnullptr=0")

set(SOURCES
    main.cpp
    App.hpp App.cpp
    Critical.hpp
    Thread.hpp Thread.cpp
    Message.hpp Message.cpp
    SmartPtr.hpp
    UDP.hpp UDP.cpp
    UDPMessenger.hpp UDPMessenger.cpp
    FrameBlaster.hpp FrameBlaster.cpp
    )

add_executable(SatBlam ${SOURCES})

target_link_libraries(SatBlam PRIVATE pthread)
