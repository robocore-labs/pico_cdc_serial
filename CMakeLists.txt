# A TinyUSB CDC interface as a serial_t. The application owns tusb_config.h,
# the USB descriptors and tusb_init(); point CDC_SERIAL_TUSB_CONFIG_DIR at
# the directory holding tusb_config.h (defaults to the top-level source dir,
# which is where firmwares in this family keep it).
set(CDC_SERIAL_TUSB_CONFIG_DIR "${CMAKE_SOURCE_DIR}" CACHE PATH
    "Directory containing the application's tusb_config.h")

add_library(pico_cdc_serial STATIC ${CMAKE_CURRENT_LIST_DIR}/src/cdc_serial.c)
target_include_directories(pico_cdc_serial PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(pico_cdc_serial PRIVATE ${CDC_SERIAL_TUSB_CONFIG_DIR})
target_link_libraries(pico_cdc_serial PUBLIC pico_serial pico_stdlib tinyusb_device)
target_compile_options(pico_cdc_serial PRIVATE -Wall -Wextra)
