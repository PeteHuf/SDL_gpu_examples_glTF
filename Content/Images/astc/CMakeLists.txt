file(RELATIVE_PATH relative_path ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

list(APPEND ASSETS_LIST
	${relative_path}/4x4.astc
	${relative_path}/5x4.astc
	${relative_path}/5x5.astc
	${relative_path}/6x5.astc
	${relative_path}/6x6.astc
	${relative_path}/8x5.astc
	${relative_path}/8x6.astc
	${relative_path}/8x8.astc
	${relative_path}/10x5.astc
	${relative_path}/10x6.astc
	${relative_path}/10x8.astc
	${relative_path}/10x10.astc
	${relative_path}/12x10.astc
	${relative_path}/12x12.astc
)

# Push the updated list back to the parent scope
set(ASSETS_LIST ${ASSETS_LIST} PARENT_SCOPE)
