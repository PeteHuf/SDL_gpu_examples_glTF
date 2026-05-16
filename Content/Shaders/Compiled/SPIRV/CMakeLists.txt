file(RELATIVE_PATH relative_path ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

list(APPEND ASSETS_LIST
	${relative_path}/CustomSampling.frag.spv
	${relative_path}/DepthOutline.frag.spv
	${relative_path}/FillTexture.comp.spv
	${relative_path}/Fullscreen.vert.spv
	${relative_path}/GradientTexture.comp.spv
	${relative_path}/LinearToSRGB.comp.spv
	${relative_path}/LinearToST2084.comp.spv
	${relative_path}/PositionColor.vert.spv
	${relative_path}/PositionColorInstanced.vert.spv
	${relative_path}/PositionColorTransform.vert.spv
	${relative_path}/PositionTexturedTransform.vert.spv
	${relative_path}/PullSpriteBatch.vert.spv
	${relative_path}/RawTriangle.vert.spv
	${relative_path}/Skybox.frag.spv
	${relative_path}/Skybox.vert.spv
	${relative_path}/SolidColor.frag.spv
	${relative_path}/SolidColorDepth.frag.spv
	${relative_path}/SpriteBatch.comp.spv
	${relative_path}/TexturedDepth.frag.spv
	${relative_path}/TexturedQuad.comp.spv
	${relative_path}/TexturedQuad.frag.spv
	${relative_path}/TexturedQuad.vert.spv
	${relative_path}/TexturedQuadArray.frag.spv
	${relative_path}/TexturedQuadColor.frag.spv
	${relative_path}/TexturedQuadColorWithMatrix.vert.spv
	${relative_path}/TexturedQuadWithMatrix.vert.spv
	${relative_path}/TexturedQuadWithMultiplyColor.frag.spv
	${relative_path}/ToneMapACES.comp.spv
	${relative_path}/ToneMapExtendedReinhardLuminance.comp.spv
	${relative_path}/ToneMapHable.comp.spv
	${relative_path}/ToneMapReinhard.comp.spv
)

# Push the updated list back to the parent scope
set(ASSETS_LIST ${ASSETS_LIST} PARENT_SCOPE)
