cmake_minimum_required(VERSION 3.13)
project(at86rf215_lib
        VERSION 1.0.0
        LANGUAGES C
        DESCRIPTION "AT86RF215 API library")

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Bring the headers
include_directories(./src)
include_directories(${PROJECT_SOURCE_DIR}/src)
# add_compile_options(-Wall -Wextra -Wno-unused-variable -Wno-missing-braces)

set(SOURCES_LIB src/at86rf215.c src/at86rf215_events.c src/at86rf215_radio.c src/at86rf215_baseband.c)
set(TARGET_LINK_LIBS io_utils zf_log rt m pthread)
set(SOURCES ${SOURCES_LIB} src/test_at86rf215.c)

# Add internal project dependencies
add_subdirectory(src/io_utils EXCLUDE_FROM_ALL)
add_subdirectory(src/zf_log EXCLUDE_FROM_ALL)  

# Create the library cariboulite
add_library(at86rf215_lib STATIC ${SOURCES_LIB})
target_link_libraries(at86rf215_lib PRIVATE ${TARGET_LINK_LIBS})
set_target_properties(at86rf215_lib PROPERTIES PUBLIC_HEADER "src/at86rf215.h;src/at86rf215_regs.h;src/at86rf215_radio.h;src/at86rf215_common.h;src/at86rf215_baseband.h")
set_target_properties(at86rf215_lib PROPERTIES OUTPUT_NAME _at86rf215_lib)

add_library(at86rf215_lib_shared SHARED ${SOURCES_LIB})
target_link_libraries(at86rf215_lib_shared PRIVATE ${TARGET_LINK_LIBS})
set_target_properties(at86rf215_lib_shared PROPERTIES PUBLIC_HEADER "src/at86rf215.h;src/at86rf215_regs.h;src/at86rf215_radio.h;src/at86rf215_common.h;src/at86rf215_baseband.h")
set_target_properties(at86rf215_lib_shared PROPERTIES OUTPUT_NAME _at86rf215_lib)

add_executable(test_at86rf215_tx ${SOURCES})
target_compile_definitions(test_at86rf215_tx PUBLIC -DTEST_TX=1)
# get_target_property(TEST_TX test_at86rf215_tx COMPILE_DEFINITIONS)
# MESSAGE( STATUS "Check if compile defs contain: " ${TEST_TX} )
target_link_libraries(test_at86rf215_tx ${TARGET_LINK_LIBS})

add_executable(test_at86rf215_rx ${SOURCES})
target_compile_definitions(test_at86rf215_rx PUBLIC -DTEST_RX=1)
# get_target_property(TEST_TX test_at86rf215_tx COMPILE_DEFINITIONS)
# MESSAGE( STATUS "Check if compile defs contain: " ${TEST_RX} )
target_link_libraries(test_at86rf215_rx ${TARGET_LINK_LIBS})

# Install targets ...
install(TARGETS test_at86rf215_rx test_at86rf215_tx at86rf215_lib_shared # zf_log io_utils
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include
        )

# Install headers from zf_log io_utils modules ...
install(FILES src/zf_log/zf_log.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zf_log)
install(FILES src/io_utils/io_utils.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/io_utils)
install(FILES src/io_utils/gpiodev_lib.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/io_utils)
install(FILES src/io_utils/spi.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/io_utils)

# make uninstall target
add_custom_target("uninstall" COMMENT "Uninstall installed files")
add_custom_command(
    TARGET "uninstall"
    POST_BUILD
    COMMENT "Uninstall files with install_manifest.txt"
    COMMAND xargs rm -vf < install_manifest.txt || echo Nothing in
            install_manifest.txt to be uninstalled!
)

# Set the location for library installation -- i.e., /usr/lib in this case
# not really necessary in this example. Use "sudo make install" to apply
# install(TARGETS test_at86rf215 DESTINATION /usr/lib)

# Install prefix if needed ...
# cmake ../ -Wno-dev -DCMAKE_INSTALL_PREFIX=/home/pavelf/gnuradio_addons/lib_at86rf215
# important !!! --> LD library search path if not installed to system ...
# export LD_LIBRARY_PATH="/home/pavelf/gnuradio_addons/lib_at86rf215/lib:$LD_LIBRARY_PATH"

# Install to default system 
# [ 14%] Built target io_utils
# [ 21%] Built target zf_log
# [ 39%] Built target at86rf215_lib_shared
# [ 57%] Built target at86rf215_lib
# [ 78%] Built target test_at86rf215_rx
# [100%] Built target test_at86rf215_tx
# Install the project...
# -- Install configuration: "Release"
# -- Installing: /usr/local/bin/test_at86rf215_rx
# -- Installing: /usr/local/bin/test_at86rf215_tx
# -- Installing: /usr/local/lib/lib_at86rf215_lib.so
# -- Installing: /usr/local/include/at86rf215.h
# -- Installing: /usr/local/include/at86rf215_regs.h
# -- Installing: /usr/local/include/at86rf215_radio.h
# -- Installing: /usr/local/include/at86rf215_common.h
# -- Installing: /usr/local/include/at86rf215_baseband.h
# -- Installing: /usr/local/include/zf_log/zf_log.h
# -- Installing: /usr/local/include/io_utils/io_utils.h
# -- Installing: /usr/local/include/io_utils/gpiodev_lib.h
# -- Installing: /usr/local/include/io_utils/spi.h


