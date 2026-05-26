cmake_minimum_required(VERSION 3.12)
project(ResourcePacker)

add_executable(ResourcePacker main.cpp)
set_target_properties(ResourcePacker PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
himii_set_output_dirs(ResourcePacker)

set(HIMII_ENGINE_HPCK "${CMAKE_BINARY_DIR}/generated/engine.hpck" CACHE INTERNAL "Packed engine resources")

add_custom_command(
    OUTPUT "${HIMII_ENGINE_HPCK}"
    COMMAND $<TARGET_FILE:ResourcePacker>
        --root "${CMAKE_SOURCE_DIR}/HimiiEditor/assets" assets
        --root "${CMAKE_SOURCE_DIR}/HimiiEditor/resources" resources
        --output "${HIMII_ENGINE_HPCK}"
    DEPENDS ResourcePacker
    COMMENT "Packing engine resources into engine.hpck"
    VERBATIM
)

add_custom_target(PackEngineResources DEPENDS "${HIMII_ENGINE_HPCK}")
