cmake_minimum_required(VERSION 3.16)
project(OpenYLM LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(OPENYLM_COMMON_SOURCES
    bytecode/encoder.cpp
    bytecode/decoder.cpp
    bytecode/format_debug.cpp
    bytecode/format_binary.cpp
    ir/ir_builder.cpp
    compiler/yaml_to_ir.cpp
    compiler/ir_to_bytecode.cpp
    compiler/module_resolver.cpp
    host/host_registry.cpp
    vm/vm.cpp
    vm/scheduler.cpp
    vm/multi_app_runtime.cpp
)

add_library(openylm_core STATIC ${OPENYLM_COMMON_SOURCES})
target_include_directories(openylm_core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(yamll
    cli/main.cpp
    cli/command_parser.cpp
    cli/compiler_dispatcher.cpp
    cli/file_loader.cpp
    cli/debug_output.cpp
    runtime/test_runner.cpp
)
target_include_directories(yamll PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(yamll PRIVATE openylm_core)

add_executable(yvm
    vm/main.cpp
)
target_include_directories(yvm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(yvm PRIVATE openylm_core)
