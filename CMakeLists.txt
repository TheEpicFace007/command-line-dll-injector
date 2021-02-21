cmake_minimum_required(VERSION 3.18)
project("CommandLineInjector")

add_executable(main "src/main.cpp"  "src/WinApiWrapper.cpp" "src/Util.cpp")
include_directories(main "include")
target_link_libraries(main "psapi.lib" "Wtsapi32.lib")

set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME ON) 
set_property(TARGET main PROPERTY CXX_STANDARD 17)
set_property(TARGET main PROPERTY CXX_EXTENSIONS ON)


target_include_directories(main PUBLIC "include")
target_include_directories(main PUBLIC "C:/Users/ASD/Project/cplusplus/lib/vcpkg/installed/x64-windows/include") # set this as the vscode intelisense won't show the path unless I do this
target_compile_definitions(main PUBLIC "UNICODE")

if(NOT Boost_INCLUDE_DIR)
  SET(Boost_INCLUDE_DIR "C:/Users/ASD/Project/cplusplus/lib/vcpkg/installed/x64-windows/include")
endif()
FIND_PACKAGE(Boost)
target_include_directories(main PUBLIC Boost_INCLUDE_DIR)

if (MSVC)
  add_compile_options("/subsystem:console")
endif()