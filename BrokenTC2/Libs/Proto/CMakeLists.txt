## Broken The Crew 2 sequential clutch assist
## Copyright (C) 2022 BrokenGameNoob <brokengamenoob@gmail.com>
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
add_library("${target_name}")

file(GLOB src_files   "*.cpp")
file(GLOB hdr_files   "*.hpp" "*.h")
file(GLOB proto_files "*.proto")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
if(protobuf_MODULE_COMPATIBLE)
    # Legacy Support
    protobuf_generate_cpp(proto_srcs proto_hdrs "${proto_files}")
    list(APPEND src_files "${proto_srcs}")
    list(APPEND hdr_files "${proto_hdrs}")
endif()


add_library(${target_name}::${target_name} ALIAS "${target_name}")

target_sources("${target_name}"
    PRIVATE 
        "${src_files}"
        "${hdr_files}"
        "${proto_files}")

# target_link_libraries("${target_name}"
#     INTERFACE
#         protobuf::libprotobuf
#         protobuf::libprotoc)
target_link_libraries("${target_name}"
    INTERFACE
    ${PROTOBUF_LIBRARIES})

if(NOT protobuf_MODULE_COMPATIBLE)
    # Modern Usage
    protobuf_generate(TARGET "${target_name}")
endif()