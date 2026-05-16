add_subdirectory(Images)
add_subdirectory(Models)
add_subdirectory(Shaders)

# Push the updated list back to the parent scope
set(ASSETS_LIST ${ASSETS_LIST} PARENT_SCOPE)