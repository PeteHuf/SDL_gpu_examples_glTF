file(RELATIVE_PATH relative_path ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

list(APPEND ASSETS_LIST
	${relative_path}/CustomSampling.frag.dxil
	${relative_path}/DepthOutline.frag.dxil
	${relative_path}/FillTexture.comp.dxil
	${relative_path}/Fullscreen.vert.dxil
	${relative_path}/GradientTexture.comp.dxil
	${relative_path}/LinearToSRGB.comp.dxil
	${relative_path}/LinearToST2084.comp.dxil
	${relative_path}/PositionColor.vert.dxil
	${relative_path}/PositionColorInstanced.vert.dxil
	${relative_path}/PositionColorTransform.vert.dxil
	${relative_path}/PositionTexturedTransform.vert.dxil
	${relative_path}/PullSpriteBatch.vert.dxil
	${relative_path}/RawTriangle.vert.dxil
	${relative_path}/Skybox.frag.dxil
	${relative_path}/Skybox.vert.dxil
	${relative_path}/SolidColor.frag.dxil
	${relative_path}/SolidColorDepth.frag.dxil
	${relative_path}/SpriteBatch.comp.dxil
	${relative_path}/TexturedDepth.frag.dxil
	${relative_path}/TexturedQuad.comp.dxil
	${relative_path}/TexturedQuad.frag.dxil
	${relative_path}/TexturedQuad.vert.dxil
	${relative_path}/TexturedQuadArray.frag.dxil
	${relative_path}/TexturedQuadColor.frag.dxil
	${relative_path}/TexturedQuadColorWithMatrix.vert.dxil
	${relative_path}/TexturedQuadWithMatrix.vert.dxil
	${relative_path}/TexturedQuadWithMultiplyColor.frag.dxil
	${relative_path}/ToneMapACES.comp.dxil
	${relative_path}/ToneMapExtendedReinhardLuminance.comp.dxil
	${relative_path}/ToneMapHable.comp.dxil
	${relative_path}/ToneMapReinhard.comp.dxil
)

# Push the updated list back to the parent scope
set(ASSETS_LIST ${ASSETS_LIST} PARENT_SCOPE)
