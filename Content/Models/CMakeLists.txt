add_subdirectory(Dumpy_bread)
add_subdirectory(Dumpy_normals)

# Push the updated list back to the parent scope
set(ASSETS_LIST ${ASSETS_LIST} PARENT_SCOPE)