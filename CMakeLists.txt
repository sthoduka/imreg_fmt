cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(imreg_fmt)

find_library(FFTW_LIBRARY
     NAMES fftw3 fftw)
set(FFTW_LIBRARIES "${FFTW_LIBRARY}")

find_library(FFTW_libm_LIBRARY
         NAMES m)
list(APPEND FFTW_LIBRARIES "${FFTW_libm_LIBRARY}")

find_package(OpenCV REQUIRED)

include_directories(
    include/
    ${OpenCV_INCLUDE_DIRS}
    ${fftw_path}
    /usr/include/eigen3
)

add_library(imreg_fmt
    src/image_dft.cpp
    src/image_transforms.cpp
    src/image_registration.cpp
)

target_link_libraries(imreg_fmt
    ${OpenCV_LIBS}
    ${FFTW_LIBRARIES}
)

add_executable(video_main
    src/video_main.cpp
)

target_link_libraries(video_main
    imreg_fmt
)

add_executable(image_main
    src/image_main.cpp
)

target_link_libraries(image_main
    imreg_fmt
)
