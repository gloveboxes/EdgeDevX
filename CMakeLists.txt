CMAKE_MINIMUM_REQUIRED(VERSION 3.8)
PROJECT(edge_devx C)

################################################################################
# Source groups
################################################################################
set(Source
    "./src/dx_avnet_iot_connect.c"
    "./src/dx_azure_iot.c"
    "./src/dx_config.c"
    "./src/dx_device_twins.c"
    "./src/dx_direct_methods.c"
    "./src/dx_json_serializer.c"
    "./src/dx_terminate.c"
    "./src/dx_timer.c"
    "./src/dx_utilities.c"
    "./src/parson.c"
)
source_group("Source" FILES ${Source})

# Enable or disable RemoteX Peripheral support
if (ENABLE_EDGE_DEVX_REMOTEX)

    set(Peripherals
        "./src/dx_gpio.c"
        "./src/dx_i2c.c"
        "./src/dx_pwm.c"
    )
    source_group("Peripherals" FILES ${Peripherals})

endif(ENABLE_EDGE_DEVX_REMOTEX)

set(ALL_FILES
    ${Source}
    ${Peripherals}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

# Enable or disable RemoteX Peripheral support
if (ENABLE_EDGE_DEVX_REMOTEX)
    message(STATUS "RemoteX Peripherals Enabled")
    # Following line required for Azure Sphere RemoteX Client library
    target_include_directories(${PROJECT_NAME} PUBLIC ../AzureSphereRemoteX )
else (ENABLE_EDGE_DEVX_REMOTEX)
    message(STATUS "RemoteX Peripherals Disabled")
endif(ENABLE_EDGE_DEVX_REMOTEX)

# Set options for Azure IoT SDK C lib
set(use_amqp OFF CACHE  BOOL "Set amqp off" FORCE )
set(use_http OFF CACHE  BOOL "Set http off" FORCE )
set(use_mqtt ON CACHE  BOOL "Set http off" FORCE )
set(skip_samples ON CACHE  BOOL "Skip samples" FORCE )
set(use_prov_client ON CACHE  BOOL "Use prov client" FORCE )

# Add Azure IoT SDK C
add_subdirectory(azure-iot-sdk-c out)


# Enable or disable RemoteX Peripheral support
if (ENABLE_EDGE_DEVX_REMOTEX)
    # ################################################################################
    # # The following line required for the AzureSphereRemoteX.Client library
    add_subdirectory("AzureSphereRemoteX" applibs)
    target_link_libraries(${PROJECT_NAME} applibs)
    # ################################################################################
endif(ENABLE_EDGE_DEVX_REMOTEX)

target_include_directories(${PROJECT_NAME} PUBLIC include)

set(ROOT_NAMESPACE rpi_libs)
set_target_properties(${PROJECT_NAME} PROPERTIES VS_GLOBAL_KEYWORD "DesktopDevX")

target_include_directories(${PROJECT_NAME} PUBLIC azure-iot-sdk-c/iothub_client/inc)
target_include_directories(${PROJECT_NAME} PUBLIC azure-iot-sdk-c/provisioning_client/inc)
target_include_directories(${PROJECT_NAME} PUBLIC azure-iot-sdk-c/c-utility/inc)
target_include_directories(${PROJECT_NAME} PUBLIC azure-iot-sdk-c/deps/umock-c/inc)
target_include_directories(${PROJECT_NAME} PUBLIC azure-iot-sdk-c/deps/azure-macro-utils-c/inc)

#
# The following config was required for macOS homebrew installed UV, UUID and SSL libs
# Install brew
# /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
#
# Install required libraries
# brew install mosquitto libuv openssl ossp-uuid
#
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # macOS on Apple silicon (arm64)
    if (${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "arm64")
        target_include_directories(${PROJECT_NAME} PUBLIC /opt/homebrew/include)
        target_link_directories(${PROJECT_NAME} PUBLIC /opt/homebrew/lib)
        target_link_directories(${PROJECT_NAME} PUBLIC /opt/homebrew/Cellar/openssl@3/3.0.1/lib/)
    else()
    # macOS on Intel
        target_include_directories(${PROJECT_NAME} PUBLIC /usr/local/include)
        target_link_directories(${PROJECT_NAME} PUBLIC /usr/local/lib)
        target_include_directories(${PROJECT_NAME} PUBLIC /usr/local/opt/openssl@3/include)
        target_link_directories(${PROJECT_NAME} PUBLIC /usr/local/opt/openssl@3/lib)
    endif()
endif()

target_link_libraries (${PROJECT_NAME}
    uv
    pthread
    iothub_client
    umqtt
    prov_auth_client
    hsm_security_client
    # uhttp
    aziotsharedutil
    parson
    uuid
    pthread
    curl
    ssl
    crypto
    m
    prov_device_ll_client
    prov_mqtt_transport
)
