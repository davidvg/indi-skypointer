cmake_minimum_required(VERSION 2.4.7)
PROJECT(skypointer CXX C)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake_modules/")

include(GNUInstallDirs)

set (GENERIC_VERSION_MAJOR 0)
set (GENERIC_VERSION_MINOR 1)

find_package(INDI REQUIRED)
find_package(Nova REQUIRED)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/indi_skypointer.xml.cmake ${CMAKE_CURRENT_BINARY_DIR}/indi_skypointer.xml )

include_directories( ${CMAKE_CURRENT_BINARY_DIR})
include_directories( ${CMAKE_CURRENT_SOURCE_DIR})
include_directories( ${INDI_INCLUDE_DIR})
include_directories( ${NOVA_INCLUDE_DIR})
include_directories( ${NOVA_INCLUDE_DIR}/..)

include(CMakeCommon)

############# SKYPOINTER ###############
set(skypointer_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/skypointer.cpp
)

add_executable(indi_skypointer ${skypointer_SRCS})

target_link_libraries(indi_skypointer ${INDI_LIBRARIES})

install(TARGETS indi_skypointer RUNTIME DESTINATION bin)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/indi_skypointer.xml DESTINATION ${INDI_DATA_DIR})
