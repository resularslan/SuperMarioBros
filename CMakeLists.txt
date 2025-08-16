# CMakeList.txt : CMake project for SuperMarioBros, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("SuperMarioBros")

set(CMAKE_CXX_STANDARD 17)
# Find SDL3
set(SDL3_DIR "C:/Users/Resul Arslan/Desktop/Şifreler ve Özeller/C++/Libraries/SDL3/cmake")
set(SDL3_IMAGE_DIR "C:/Users/Resul Arslan/Desktop/Şifreler ve Özeller/C++/Libraries/SDL3_image-3.2.4/cmake")
find_package(SDL3 REQUIRED)
find_package(SDL3_IMAGE REQUIRED)
# Add source to this project's executable.
add_executable (SuperMarioBros "scripts/main.cpp")
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET SuperMarioBros PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
target_link_libraries(SuperMarioBros PRIVATE SDL3_image::SDL3_image SDL3::SDL3)
