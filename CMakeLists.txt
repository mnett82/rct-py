cmake_minimum_required(VERSION 3.10) # Or a newer version you prefer

project(rct CXX)

add_library(rct SHARED bindings.cc rct.cc random.cc)

install(TARGETS ${PROJECT_NAME}
        DESTINATION ${CMAKE_INSTALL_LIBDIR})
