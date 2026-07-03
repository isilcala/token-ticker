cmake_minimum_required(VERSION 3.22)

set(IDF_TARGET esp32s3 CACHE STRING "ESP-IDF target" FORCE)

set(EXTRA_COMPONENT_DIRS
    ../app
    ../bsp/boards/waveshare-s3-rlcd-4.2
    ../bsp/display
    ../bsp/power
    ../bsp/sensor
    ../domain
    ../platform/http
    ../platform/storage
    ../platform/telemetry
    ../platform/time
    ../platform/wifi
    ../providers/provider_registry
    ../providers/minimax
    ../ui
)

set(TEST_COMPONENTS
    app
    minimax
    storage
    ui
    CACHE STRING "List of components to test" FORCE
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(MINIMAL_BUILD ON)
project(token_ticker_test)