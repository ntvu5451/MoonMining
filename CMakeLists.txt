﻿# CMakeList.txt : CMake project for MoonMiningProject, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("MoonMiningProject")

# Add source to this project's library used for Unit Tests.
add_library (MoonProject "MoonProject.cpp" "MoonMining.cpp" "MoonMining.h")

#Add source to the project's executable
add_executable(MoonProjectTarget "MoonProject.cpp" "MoonMining.cpp" "MoonMining.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET MoonProject PROPERTY CXX_STANDARD 20)
  set_property(TARGET MoonProjectTarget PROPERTY CXX_STANDARD 20)
endif()

