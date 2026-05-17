add_subdirectory(astc)
add_subdirectory(bcn)

file(RELATIVE_PATH relative_path ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

list(APPEND ASSETS_LIST
	${relative_path}/armadillo.png
	${relative_path}/cube0.bmp
	${relative_path}/cube0mip1.bmp
	${relative_path}/cube1.bmp
	${relative_path}/cube1mip1.bmp
	${relative_path}/cube2.bmp
	${relative_path}/cube2mip1.bmp
	${relative_path}/cube3.bmp
	${relative_path}/cube3mip1.bmp
	${relative_path}/cube4.bmp
	${relative_path}/cube5.bmp
	${relative_path}/flat_n.bmp
	${relative_path}/latency.bmp
	${relative_path}/memorial.hdr
	${relative_path}/ravioli.bmp
	${relative_path}/ravioli_atlas.bmp
	${relative_path}/ravioli_inverted.bmp
)

# Push the updated list back to the parent scope
set(ASSETS_LIST ${ASSETS_LIST} PARENT_SCOPE)
