cmake_minimum_required(VERSION 2.8)
include(ExternalProject)

list(APPEND CMAKE_MODULE_PATH  "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(tutorial)

  find_package(PythonInterp REQUIRED)
  find_package(PythonLibs REQUIRED)
  include(BuildBoost)

  include_directories(${Boost_INCLUDE_DIR} ${PYTHON_INCLUDE_DIRS})
  link_directories(${Boost_LIBRARY_DIR})

  add_library(pylib SHARED pylib.cpp sidebook.cpp orderbook.cpp)
  target_link_libraries(pylib ${Boost_LIBRARIES} ${PYTHON_LIBRARIES})
  add_dependencies(pylib Boost)

  set_target_properties(pylib PROPERTIES SUFFIX .so)
  set_target_properties(pylib PROPERTIES PREFIX "")
