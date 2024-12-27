cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

cmake_policy(SET CMP0076 NEW)

project ("DPN_Generator")

# Uncomment if you want to build with verbose output
#add_compile_definitions(DEBUG)

add_executable (DPN_Generator "main.cpp" "network_gen.cpp" "actor_gen.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET DPN_Generator PROPERTY CXX_STANDARD 20)
endif()

set_property(TARGET DPN_Generator PROPERTY CXX_STANDARD_REQUIRED 17)

target_include_directories(DPN_Generator PUBLIC ${CMAKE_CURRENT_LIST_DIR})


