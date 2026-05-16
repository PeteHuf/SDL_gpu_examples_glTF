file(RELATIVE_PATH relative_path ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

list(APPEND ASSETS_LIST
	${relative_path}/DamagedHelmet.gltf
)

# Push the updated list back to the parent scope
set(ASSETS_LIST ${ASSETS_LIST} PARENT_SCOPE)