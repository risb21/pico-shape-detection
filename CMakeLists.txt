cmake_minimum_required(VERSION 3.12)

# set(PICO_BOARD pico_w)
include(pico_sdk_import.cmake)

project(pico_acc C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(pico_acc
	${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/MPU6050.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/tflite_wrapper.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/shape_model.hpp
)


target_include_directories(pico_acc PRIVATE
	${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(pico_acc
	pico_stdlib
	pico-tflmicro
	hardware_i2c
	hardware_gpio
	pico_cyw43_arch_lwip_threadsafe_background
)

pico_enable_stdio_usb(pico_acc 1)
pico_enable_stdio_uart(pico_acc 0)

pico_add_extra_outputs(pico_acc)

add_subdirectory("lib/tflmicro" EXCLUDE_FROM_ALL)