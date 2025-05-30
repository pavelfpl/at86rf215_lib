cmake_minimum_required(VERSION 3.13)
project(at86rf215_lib)
set(CMAKE_BUILD_TYPE Release)

# Bring the headers, such as Student.h into the project
set(SUPER_DIR ${PROJECT_SOURCE_DIR}/..)
include_directories(/.)
include_directories(${SUPER_DIR})

#However, the file(GLOB...) allows for wildcard additions:
set(SOURCES_LIB spi.c gpiodev_lib.c io_utils.c)

# set(EXTERN_LIBS ${SUPER_DIR}/zf_log/build/libzf_log.a)
# add_compile_options(-Wall -Wextra -pedantic -Werror)
# add_compile_options(-Wall -Wextra)
add_compile_options(-Wno-missing-braces -Wno-format-zero-length -Wall)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# Generate the static library from the sources
add_library(io_utils STATIC ${SOURCES_LIB} )
target_include_directories(io_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(io_utils PRIVATE pthread)

add_executable(test_io_utils spi_test_custom_api.c)
target_link_libraries(test_io_utils io_utils pthread) # ${EXTERN_LIBS}

# Set the location for library installation -- i.e., /usr/lib in this case
# not really necessary in this example. Use "sudo make install" to apply
install(TARGETS io_utils DESTINATION /usr/lib)
