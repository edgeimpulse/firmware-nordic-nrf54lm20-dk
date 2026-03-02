#
# Copyright (c) 2025 Edge Impulse
#
cmake_minimum_required(VERSION 3.20.0)

set(EXTRA_CONF_FILE
        ./ei-model/conf_overlay.conf
)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project("firmware-nordic-nrf54lm20-dk"
          VERSION 0.1)

# Needed for colorful output
zephyr_compile_options(-fdiagnostics-color=always)
zephyr_compile_options(-Wno-narrowing)

# Use hardware acceleration for DSP and Neural Network code
# You'll need to disable these on non-Arm cores
add_definitions(-DEIDSP_USE_CMSIS_DSP=1
                -DEIDSP_LOAD_CMSIS_DSP_SOURCES=1
                -DEI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN=1
                -DEIDSP_QUANTIZE_FILTERBANK=0
                -DARM_MATH_LOOPUNROLL
                -DMBEDTLS_PLATFORM_ZEROIZE_ALT
                )

# Add all required source files
add_subdirectory(ei-model/edge-impulse-sdk/cmake/zephyr)
add_subdirectory(firmware-sdk)
add_subdirectory(src/inference)
add_subdirectory(src/sensors)
add_subdirectory(src)

# Include directories (everything in the SDK is already included here)
target_include_directories(app PRIVATE .)
target_include_directories(app PRIVATE ei-model)
zephyr_include_directories(ei-model/nordic-axon-model)

#
# model name is specified through kConfig. However for automated builds the
# environment variable AXON_MODEL_NAME can override kConfig.
#
if (DEFINED ENV{NRF_AXON_MODEL_NAME})
    message(STATUS "NRF_AXON_MODEL_NAME DEFINED $ENV{NRF_AXON_MODEL_NAME}, OVERRIDES CONFIG_NRF_AXON_MODEL_NAME ${CONFIG_NRF_AXON_MODEL_NAME}")
    zephyr_compile_definitions(
        NRF_AXON_MODEL_NAME=$ENV{NRF_AXON_MODEL_NAME}
    )
else()
    message(STATUS "NRF_AXON_MODEL_NAME NOT DEFINED, USING CONFIG_NRF_AXON_MODEL_NAME ${CONFIG_NRF_AXON_MODEL_NAME}")
    zephyr_compile_definitions(
        NRF_AXON_MODEL_NAME=${CONFIG_NRF_AXON_MODEL_NAME}
    )
endif()
