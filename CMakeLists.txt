cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(pico_acc C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_BOARD pico_w)

pico_sdk_init()

add_executable(pico_acc
	${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
	# ${CMAKE_CURRENT_LIST_DIR}/src/ml_model.cpp
)

target_link_libraries(pico_acc
	pico_stdlib
	hardware_i2c
)

pico_enable_stdio_usb(pico_acc 1)
pico_enable_stdio_uart(pico_acc 0)

pico_add_extra_outputs(pico_acc)

# example_auto_set_url(pico_acc)
