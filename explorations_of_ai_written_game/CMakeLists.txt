cmake_minimum_required(VERSION 3.21)
project(untitled6 C CXX)


enable_language(ASM_NASM)


set(CMAKE_C_STANDARD 11)

set(ASM_SOURCES
        test.asm
        )

set (SOURCES
        #main.c
        #${ASM_SOURCES}
        test.c)

set_source_files_properties(${ASM_SOURCES} PROPERTIES LANGUAGE ASM_NASM)

add_executable(program  ${SOURCES})