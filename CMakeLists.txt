cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0069 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

set(VERBOSE ON)

# Get current commit hash as GIT_COMMIT
find_package(Git)
set(GIT_COMMIT "unknown") # Default value for when Git is not found
if (EXISTS "${CMAKE_SOURCE_DIR}/.git/index" AND GIT_FOUND)
    execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD RESULT_VARIABLE GIT_GET_HASH_RESULT OUTPUT_VARIABLE GIT_COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(GIT_GET_HASH_RESULT)
        message(STATUS "You are currently on commit ${GIT_COMMIT}")
    else()
        message(WARNING "Cannot determine current commit. Make sure that you are building either from a Git working tree or from a source archive.")
    endif()
endif()

# Read the package.json file and extract the top-level .version as LIBSESSION_NODEJS_VERSION
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/package.json" PACKAGE_JSON)
string(JSON LIBSESSION_NODEJS_VERSION GET "${PACKAGE_JSON}" version)

# Generate src/version.h
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/src/version.h"
"const char* LIBSESSION_NODEJS_VERSION = \"${LIBSESSION_NODEJS_VERSION}\";
const char* LIBSESSION_NODEJS_COMMIT = \"${GIT_COMMIT}\";\n"
)

# Detect the number of processors
include(ProcessorCount)
ProcessorCount(N)

# Set a default value in case the detection fails
if(NOT N EQUAL 0)
  set(CMAKE_BUILD_PARALLEL_LEVEL ${N})
else()
  set(CMAKE_BUILD_PARALLEL_LEVEL 4)  # Fallback to 16 if detection fails
endif()
message(STATUS "Number of processors detected: ${N}")

add_definitions(-DNAPI_VERSION=8)
set(CMAKE_CONFIGURATION_TYPES Release)



project(libsession_util_nodejs LANGUAGES CXX)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_BUILD_TYPE Release)
SET(WITH_TESTS OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(ENABLE_ONIONREQ OFF)

# when building from a release of libsession on desktop, it complains that ios-cmake is not up to date
# as it is not part of the archive. We actually don't care about it on session-desktop
set(SUBMODULE_CHECK OFF)

file(GLOB SOURCE_FILES src/*.cpp src/groups/*.cpp src/encrypt_decrypt/*.cpp src/pro/*.cpp src/meta/*.cpp )

# **not** having this allows the system library to be used.
# this breaks the release on macos, see https://github.com/session-foundation/session-desktop/issues/1844
set(OXEN_LOGGING_FORCE_SUBMODULES ON CACHE INTERNAL "")

add_subdirectory(libsession-util)

if(MSVC)
  # Windows is horrible
  add_compile_definitions(NOMINMAX)
endif()



add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})

# Mark the node-addon-api headers as system so the -Werror=switch-enum does not apply to them
target_include_directories(${PROJECT_NAME}
    SYSTEM PRIVATE
    ${CMAKE_JS_INC}
    ${CMAKE_CURRENT_SOURCE_DIR}/node_modules/node-addon-api
    ${CMAKE_CURRENT_SOURCE_DIR}/node_modules
    ${CMAKE_CURRENT_SOURCE_DIR}/node_modules/node-api-headers/include
    "../../node_modules/node-addon-api"
    "../../node_modules"
    "../../node_modules/node-api-headers/include"
)

target_include_directories(${PROJECT_NAME} PRIVATE  "include/" )

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_JS_LIB} ${LIBSESSION_STATIC_BUNDLE_LIBS})


if(UNIX AND NOT APPLE)
    target_compile_options(${PROJECT_NAME} PRIVATE -Werror=switch-enum)
endif()


if(MSVC AND CMAKE_JS_NODELIB_DEF AND CMAKE_JS_NODELIB_TARGET)
  # Generate node.lib
  execute_process(COMMAND ${CMAKE_AR} /def:${CMAKE_JS_NODELIB_DEF} /out:${CMAKE_JS_NODELIB_TARGET} ${CMAKE_STATIC_LINKER_FLAGS})
endif()
