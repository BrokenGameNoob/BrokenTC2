set(target_name "TextToSpeech")
message("==== TARGET: ${target_name}")

file(GLOB src_files   "*.cpp")
file(GLOB hdr_files   "*.hpp" "*.h")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library("${target_name}")
add_library(${target_name}::${target_name} ALIAS "${target_name}")

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS TextToSpeech)

set(ADDITIONAL_SOURCES

)

target_sources("${target_name}"
    PRIVATE 
        "${ADDITIONAL_SOURCES}"
        "${src_files}"
        "${hdr_files}"
        "${proto_files}"
)
target_link_libraries("${target_name}"
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    PUBLIC
    Qt${QT_VERSION_MAJOR}::TextToSpeech
)

target_include_directories(${target_name}
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_compile_features(${target_name} PRIVATE cxx_auto_type)
target_compile_options(${target_name} PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
    -Wall -Wextra -Wpedantic>)

message("====> END: ${target_name}")
