cmake_minimum_required(VERSION 3.20)

if(NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/arm-none-eabi.cmake)
endif()

project(stm32-baremetal C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(MCU_FLAGS -mcpu=cortex-m0plus -mthumb)
set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/linker/STM32G071RBTx_FLASH.ld)

add_executable(firmware
    startup/startup_stm32g071xx.s
    startup/system_stm32g0xx.c
    src/main.c
)

target_include_directories(firmware PRIVATE
    third_party/cmsis/core/Include
    third_party/cmsis/device/Include
    drivers
    src
)

target_compile_definitions(firmware PRIVATE STM32G071xx)

target_compile_options(firmware PRIVATE
    ${MCU_FLAGS}
    -ffunction-sections -fdata-sections
    $<$<COMPILE_LANGUAGE:C>:-Wall -Wextra -Werror -Wshadow -Wconversion>
    $<$<CONFIG:Debug>:-Og -g3>
    $<$<CONFIG:Release>:-O2 -g>
)

# Vendor files are not held to our warning rules.
set_source_files_properties(startup/system_stm32g0xx.c
    PROPERTIES COMPILE_OPTIONS "-Wno-error;-Wno-conversion")

target_link_options(firmware PRIVATE
    ${MCU_FLAGS}
    -T${LINKER_SCRIPT}
    --specs=nano.specs --specs=nosys.specs
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/firmware.map
    -Wl,--print-memory-usage
)
set_target_properties(firmware PROPERTIES
    SUFFIX ".elf"
    LINK_DEPENDS ${LINKER_SCRIPT}
)

add_custom_command(TARGET firmware POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:firmware> ${CMAKE_CURRENT_BINARY_DIR}/firmware.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:firmware> ${CMAKE_CURRENT_BINARY_DIR}/firmware.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:firmware>
)
