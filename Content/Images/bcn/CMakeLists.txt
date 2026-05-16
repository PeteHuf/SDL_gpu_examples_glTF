file(RELATIVE_PATH relative_path ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

list(APPEND ASSETS_LIST
	${relative_path}/BC1.dds
	${relative_path}/BC1_SRGB.dds
	${relative_path}/BC2.dds
	${relative_path}/BC2_SRGB.dds
	${relative_path}/BC3.dds
	${relative_path}/BC3_SRGB.dds
	${relative_path}/BC4.dds
	${relative_path}/BC5.dds
	${relative_path}/BC6H_S.dds
	${relative_path}/BC6H_U.dds
	${relative_path}/BC7.dds
	${relative_path}/BC7_SRGB.dds
)

# Push the updated list back to the parent scope
set(ASSETS_LIST ${ASSETS_LIST} PARENT_SCOPE)
