cmake_minimum_required(VERSION 3.24)

project(
  accounting
  VERSION 1.0
  LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 11)

file(GLOB HEADERS *.h)
file(GLOB SRCS *.cc)

include_directories(headers ${CMAKE_CURRENT_SOURCE_DIR})
add_library(accounting SHARED ${SRCS})
