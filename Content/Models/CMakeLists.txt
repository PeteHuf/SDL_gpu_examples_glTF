add_subdirectory(Box)
add_subdirectory(DamagedHelmet)
add_subdirectory(Dumpy_bread)
add_subdirectory(Dumpy_normals)
add_subdirectory(green_cube)

# Push the updated list back to the parent scope
set(ASSETS_LIST ${ASSETS_LIST} PARENT_SCOPE)