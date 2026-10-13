# CMakeList.txt : CMake project for EisbearsModRuntime, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("EisbearsModRuntime")

# FIX: Changed from add_executable to add_library SHARED so it outputs .dll / .so
add_library (EisbearsModRuntime SHARED "EisbearsModRuntime.cpp" "EisbearsModRuntime.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET EisbearsModRuntime PROPERTY CXX_STANDARD 20)
  set_property(TARGET EisbearsModRuntime PROPERTY CXX_STANDARD_REQUIRED ON)
endif()
