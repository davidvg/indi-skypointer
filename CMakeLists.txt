################ SkyPointer INDI Driver ################

PROJECT(skypointer CXX C)
cmake_minimum_required(VERSION 2.8)

include(GNUInstallDirs)
include (TestBigEndian)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake_modules/")

TEST_BIG_ENDIAN(IS_BIG_ENDIAN)

find_package(INDI REQUIRED)
find_package(Nova REQUIRED)

set (GENERIC_VERSION_MAJOR 0)
set (GENERIC_VERSION_MINOR 1)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/indi_skypointer.xml.cmake ${CMAKE_CURRENT_BINARY_DIR}/indi_skypointer.xml )

include_directories( ${CMAKE_CURRENT_BINARY_DIR})
include_directories( ${CMAKE_CURRENT_SOURCE_DIR})
include_directories( ${INDI_INCLUDE_DIR})
include_directories( ${NOVA_INCLUDE_DIR})

include(CMakeCommon)

add_executable(indi_skypointer skypointer.cpp skypointer_driver.cpp)
target_link_libraries(indi_skypointer ${INDI_LIBRARIES} ${NOVA_LIBRARIES})
install(TARGETS indi_skypointer RUNTIME DESTINATION bin)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/indi_skypointer.xml DESTINATION ${INDI_DATA_DIR})
