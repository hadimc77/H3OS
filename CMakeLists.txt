cmake_minimum_required(VERSION 3.20)
project(H3OS C ASM_NASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

enable_language(ASM_NASM)

set(H3OS_ROOT ${CMAKE_SOURCE_DIR})

add_compile_options(
    $<$<COMPILE_LANGUAGE:C>:-ffreestanding>
    $<$<COMPILE_LANGUAGE:C>:-fno-builtin>
    $<$<COMPILE_LANGUAGE:C>:-fno-stack-protector>
    $<$<COMPILE_LANGUAGE:C>:-fno-pic>
    $<$<COMPILE_LANGUAGE:C>:-mno-red-zone>
    $<$<COMPILE_LANGUAGE:C>:-Wall>
    $<$<COMPILE_LANGUAGE:C>:-Wextra>
    $<$<COMPILE_LANGUAGE:C>:-O2>
)

include_directories(
    ${H3OS_ROOT}/sdk/include
    ${H3OS_ROOT}
    ${H3OS_ROOT}/boot
)

set(CMAKE_NASM_OBJECT_FORMAT elf64)
set(CMAKE_ASM_NASM_FLAGS "-f elf64")

# Prefer the Makefile for full ISO/QEMU flow; CMake builds the ELF.
file(GLOB_RECURSE H3OS_C_SOURCES
    kernel/*.c
    memory/*.c
    drivers/*.c
    filesystem/vfs/*.c
    window_manager/*.c
    desktop/*.c
    applications/terminal/*.c
    network/*.c
    security/*.c
)

set(H3OS_ASM_SOURCES
    boot/boot.asm
    kernel/arch/x86_64/isr.asm
)

add_executable(h3os.elf ${H3OS_C_SOURCES} ${H3OS_ASM_SOURCES})
set_target_properties(h3os.elf PROPERTIES
    LINK_FLAGS "-nostdlib -static -T ${H3OS_ROOT}/boot/linker.ld"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

message(STATUS "H3OS — The Future Starts Here.")
message(STATUS "Primary build path: make / make iso / make run")
