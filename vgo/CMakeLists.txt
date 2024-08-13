project(vgo)

# Get source and header files
file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "include/*.h")

set(PROJECT_NAME "vgo")
# Add source to this project's library.
add_library(${PROJECT_NAME} SHARED)
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES} ${HEADERS})
include_directories(${PROJECT_NAME} PUBLIC include)
target_compile_definitions(${PROJECT_NAME} PRIVATE VGO_EXPORT)


find_package(glad CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glad::glad)
find_package(glm CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                           ${CMAKE_CURRENT_SOURCE_DIR}/glsl/
                           ${CMAKE_BINARY_DIR}/Viewer.Avalonia.Entry/glsl)

file(GLOB VGO_LIBS "${CMAKE_CURRENT_BINARY_DIR}/*.dll")
file(GLOB VGO_PDBS "${CMAKE_CURRENT_BINARY_DIR}/*.pdb")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                           ${VGO_LIBS}
                           ${VGO_PDBS}
                           ${CMAKE_BINARY_DIR}/Viewer.Avalonia.Entry)
