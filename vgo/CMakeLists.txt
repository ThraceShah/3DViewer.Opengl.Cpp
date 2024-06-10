set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
# Get source and header files
file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "include/*.h")

set(PROJECT_NAME "vgo")
# Add source to this project's library.
add_library(${PROJECT_NAME} SHARED)
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES} ${HEADERS})
include_directories(${PROJECT_NAME} PUBLIC include)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
endif()

find_package(glad CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glad::glad)
find_package(glm CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)


add_custom_command(TARGET vgo POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                           ${CMAKE_CURRENT_SOURCE_DIR}/glsl/
                           ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/glsl)
