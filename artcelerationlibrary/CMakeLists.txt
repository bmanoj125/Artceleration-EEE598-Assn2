#Set the minimume version of CMake needed to build the libraries for the image transform

#Format : cmake_minimum_required(VERSION major.minor[.patch[.tweak]][FATAL_ERROR])

cmake_minimum_required(VERSION 3.4.1)

#Specify the library and whether it is static or shared


add_library( # Specifies the name of the library.
             AllFilters-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/MotionBlur.cpp
             src/main/cpp/GaussianBlur.cpp
             src/main/cpp/SobelFilter.cpp
             src/main/cpp/UnsharpMask.cpp
             src/main/cpp/AllImageTransforms.cpp)



include_directories(src/main/cpp/include/)

target_link_libraries(AllFilters-lib
                      -ljnigraphics
                      android
                      log
                )
