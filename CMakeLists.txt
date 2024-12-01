cmake_minimum_required(VERSION 3.26)
project(Computer_Enhance_follow_up)

set(CMAKE_CXX_STANDARD 17)

include_directories(handlers)
include_directories(LUT)

add_executable(Computer_Enhance_follow_up
        main.cpp
        handlers/InstructionsHandlers.h
        LUT/LUT.h
        handlers/InstructionsHandlers.cpp
        handlers/InstructionSimulatorHandler.h
        handlers/InstructionSimulatorHandler.cpp)