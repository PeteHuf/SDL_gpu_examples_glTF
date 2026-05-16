add_subdirectory(DXIL)
add_subdirectory(MSL)
add_subdirectory(SPIRV)

# Push the updated list back to the parent scope
set(ASSETS_LIST ${ASSETS_LIST} PARENT_SCOPE)
