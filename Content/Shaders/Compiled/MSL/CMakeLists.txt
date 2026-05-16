file(RELATIVE_PATH relative_path ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

list(APPEND ASSETS_LIST
	${relative_path}/CustomSampling.frag.msl
	${relative_path}/DepthOutline.frag.msl
	${relative_path}/FillTexture.comp.msl
	${relative_path}/Fullscreen.vert.msl
	${relative_path}/GradientTexture.comp.msl
	${relative_path}/LinearToSRGB.comp.msl
	${relative_path}/LinearToST2084.comp.msl
	${relative_path}/PositionColor.vert.msl
	${relative_path}/PositionColorInstanced.vert.msl
	${relative_path}/PositionColorTransform.vert.msl
	${relative_path}/PositionTexturedTransform.vert.msl
	${relative_path}/PullSpriteBatch.vert.msl
	${relative_path}/RawTriangle.vert.msl
	${relative_path}/Skybox.frag.msl
	${relative_path}/Skybox.vert.msl
	${relative_path}/SolidColor.frag.msl
	${relative_path}/SolidColorDepth.frag.msl
	${relative_path}/SpriteBatch.comp.msl
	${relative_path}/TexturedDepth.frag.msl
	${relative_path}/TexturedQuad.comp.msl
	${relative_path}/TexturedQuad.frag.msl
	${relative_path}/TexturedQuad.vert.msl
	${relative_path}/TexturedQuadArray.frag.msl
	${relative_path}/TexturedQuadColor.frag.msl
	${relative_path}/TexturedQuadColorWithMatrix.vert.msl
	${relative_path}/TexturedQuadWithMatrix.vert.msl
	${relative_path}/TexturedQuadWithMultiplyColor.frag.msl
	${relative_path}/ToneMapACES.comp.msl
	${relative_path}/ToneMapExtendedReinhardLuminance.comp.msl
	${relative_path}/ToneMapHable.comp.msl
	${relative_path}/ToneMapReinhard.comp.msl
)

# Push the updated list back to the parent scope
set(ASSETS_LIST ${ASSETS_LIST} PARENT_SCOPE)
