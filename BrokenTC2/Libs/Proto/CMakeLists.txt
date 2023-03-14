# https://discourse.cmake.org/t/protobuf-generate-cpp-not-found/7053

#Expect the following to be in the main CMakeLists.txt (it's a hack but meh :/)
###############################################################################
# #See README for more info about installation
# set(Protobuf_PREFIX_PATH
#     "${BASE_PROTO_PATH}\\include"            
#     "${BASE_PROTO_PATH}\\lib"             
#     "${BASE_PROTO_PATH}\\bin" )
# list(APPEND CMAKE_PREFIX_PATH "${Protobuf_PREFIX_PATH}")

# set(Protobuf_USE_STATIC_LIBS ON)
# find_package(Protobuf REQUIRED HINTS "${BASE_PROTO_PATH}\\lib\\cmake\\protobuf")

# message(STATUS "Protobuf include dir: ${Protobuf_INCLUDE_DIRS}")

# if(NOT DEFINED Protobuf_INCLUDE_DIRS)
#     set(Protobuf_INCLUDE_DIRS "${BASE_PROTO_PATH}\\include" CACHE BOOL "")
# endif()
# include_directories(SYSTEM ${Protobuf_INCLUDE_DIRS})

# find_package(Protobuf)
# include_directories(SYSTEM ${Protobuf_INCLUDE_DIRS})
###############################################################################

set(target_name "BTC2Proto")
message("==== TARGET: ${target_name}")

file(GLOB src_files   "*.cpp")
file(GLOB hdr_files   "*.hpp" "*.h")
file(GLOB proto_files "*.proto")

set(CMAKE_INCLUDE_CURRENT_DIR ON)


add_library("${target_name}")
add_library(${target_name}::${target_name} ALIAS "${target_name}")

target_sources("${target_name}"
    PRIVATE 
        "${src_files}"
        "${hdr_files}"
        "${proto_files}")

target_link_libraries("${target_name}"
    INTERFACE
    ${PROTOBUF_LIBRARIES}
)
target_link_libraries("${target_name}"
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

set(PROTO_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")

protobuf_generate(TARGET "${target_name}" PROTOC_OUT_DIR "${PROTO_BINARY_DIR}")

target_include_directories(${target_name} PUBLIC "$<BUILD_INTERFACE:${PROTO_BINARY_DIR}>")
