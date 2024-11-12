add_executable(sandbox)
aux_source_directory(./ SANDBOX_SRC)
target_sources(sandbox PRIVATE ${SANDBOX_SRC})
target_link_libraries(sandbox PUBLIC toy2d SDL2)

# file(GLOB SPV ${PROJECT_SOURCE_DIR}/*.spv)
# add_custom_command(
#   TARGET sandbox
#   POST_BUILD
#   COMMAND ${CMAKE_COMMAND} -E copy
#   ${SPV}
#   ./
# )
add_custom_command(
  TARGET sandbox
  COMMAND glslc
  ARGS ${PROJECT_SOURCE_DIR}/shader/shader.vert -o ${CMAKE_CURRENT_BINARY_DIR}/vert.spv

  COMMAND glslc
  ARGS ${PROJECT_SOURCE_DIR}/shader/shader.frag -o ${CMAKE_CURRENT_BINARY_DIR}/frag.spv
)
CopyDLL(sandbox)
