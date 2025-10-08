cmake_minimum_required(VERSION 4.0.1)
project(blobit LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

file(GLOB_RECURSE SOURCE_FILES lib/*.c)
file(GLOB_RECURSE HEADER_FILES include/*.h)

add_library(blobit STATIC ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(blobit PUBLIC include)

if(APPLE)
    target_compile_options(blobit PRIVATE
            -Wall -Wextra -Wpedantic
            -g
            -O2
            -fno-omit-frame-pointer
            -Rpass=all
    )
    set_target_properties(blobit PROPERTIES
            MACOSX_BUNDLE TRUE
            XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym"
    )
else()
    target_compile_options(blobit PRIVATE -Wall -Wextra -Wpedantic -g -O2 -fno-omit-frame-pointer)
endif()

file(GLOB_RECURSE EXECUTABLE_FILES src/*.c)

add_executable(blobit-cli ${EXECUTABLE_FILES} ${HEADER_FILES})
target_include_directories(blobit-cli PRIVATE include)
target_link_libraries(blobit-cli PRIVATE blobit)
target_compile_options(blobit-cli PRIVATE
        -Wall -Wextra -Wpedantic
        $<$<PLATFORM_ID:Darwin>:-g -O2 -fno-omit-frame-pointer -Rpass=all>
)
set_target_properties(blobit-cli PROPERTIES OUTPUT_NAME "blobit")

file(GLOB_RECURSE BENCHMARK_FILES benchmarks/*.c)

add_executable(blobit-bench ${BENCHMARK_FILES} ${HEADER_FILES})
target_include_directories(blobit-bench PRIVATE include)
target_link_libraries(blobit-bench PRIVATE blobit)
target_compile_options(blobit-bench PRIVATE -Wall -Wextra -Wpedantic)
set_target_properties(blobit-bench PROPERTIES OUTPUT_NAME "blobit-bench")
