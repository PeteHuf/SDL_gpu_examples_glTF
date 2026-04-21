#include <array>

extern "C" {

#include "Common.h"

} // extern "C"

static SDL_GPUGraphicsPipeline* ScenePipeline;
static SDL_GPUBuffer* SceneVertexBuffer;
static SDL_GPUBuffer* SceneIndexBuffer;
static SDL_GPUTexture* SceneDepthTexture;

static float Time;
static int SceneWidth, SceneHeight;

static int Init_cpp(Context* context)
{
	int result = CommonInit(context, 0);
	if (result < 0)
	{
		return result;
	}

	// Creates the Shaders & Pipelines
	{
		SDL_GPUShader* sceneVertexShader = LoadShader(context->Device, "PositionColorTransform.vert", 0, 1, 0, 0);
		if (sceneVertexShader == nullptr)
		{
			SDL_Log("Failed to create 'PositionColorTransform' vertex shader!");
			return -1;
		}

		SDL_GPUShader* sceneFragmentShader = LoadShader(context->Device, "SolidColorDepth.frag", 0, 1, 0, 0);
		if (sceneFragmentShader == nullptr)
		{
			SDL_Log("Failed to create 'SolidColorDepth' fragment shader!");
			return -1;
		}

		std::array<SDL_GPUVertexBufferDescription, 1> sceneVertexBufferDescription{{{
			.slot = 0,
			.pitch = sizeof(PositionColorVertex),
			.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
			.instance_step_rate = 0
		}}};
		std::array<SDL_GPUVertexAttribute, 2> sceneVertexAttribute{{{
			.location = 0,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
			.offset = 0
		}, {
			.location = 1,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM,
			.offset = sizeof(float) * 3
		}}};
		std::array<SDL_GPUColorTargetDescription, 1> sceneColorTargetDescription{{{
			.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM
		}}};
		SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {
			.vertex_shader = sceneVertexShader,
			.fragment_shader = sceneFragmentShader,
			.vertex_input_state = SDL_GPUVertexInputState{
				.vertex_buffer_descriptions = sceneVertexBufferDescription.data(),
				.num_vertex_buffers = 1,
				.vertex_attributes = sceneVertexAttribute.data(),
				.num_vertex_attributes = 2
			},
			.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
			.rasterizer_state = SDL_GPURasterizerState{
				.fill_mode = SDL_GPU_FILLMODE_FILL,
				.cull_mode = SDL_GPU_CULLMODE_NONE,
				.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE
			},
			.depth_stencil_state = SDL_GPUDepthStencilState{
				.compare_op = SDL_GPU_COMPAREOP_LESS,
				.write_mask = 0xFF,
				.enable_depth_test = true,
				.enable_depth_write = true,
				.enable_stencil_test = false
			},
			.target_info = {
				.color_target_descriptions = sceneColorTargetDescription.data(),
				.num_color_targets = 1,
				.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
				.has_depth_stencil_target = true
			}
		};
		ScenePipeline = SDL_CreateGPUGraphicsPipeline(context->Device, &pipelineCreateInfo);
		if (ScenePipeline == nullptr)
		{
			SDL_Log("Failed to create Scene pipeline!");
			return -1;
		}

		SDL_ReleaseGPUShader(context->Device, sceneVertexShader);
		SDL_ReleaseGPUShader(context->Device, sceneFragmentShader);
	}

	// Create the Scene Textures
	{
		int w, h;
		SDL_GetWindowSizeInPixels(context->Window, &w, &h);
		SceneWidth = w;
		SceneHeight = h;

		SDL_GPUTextureCreateInfo depthTextureCreateInfo {
			.type = SDL_GPU_TEXTURETYPE_2D,
			.format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
			.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
			.width = static_cast<Uint32>(SceneWidth),
			.height = static_cast<Uint32>(SceneHeight),
			.layer_count_or_depth = 1,
			.num_levels = 1,
			.sample_count = SDL_GPU_SAMPLECOUNT_1
		};
		SceneDepthTexture = SDL_CreateGPUTexture(
			context->Device,
			&depthTextureCreateInfo
		);
	}

	// Create & Upload Scene Index and Vertex Buffers
	{
		SDL_GPUBufferCreateInfo sceneVertexBufferCreateInfo {
			.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
			.size = sizeof(PositionColorVertex) * 24
		};
		SceneVertexBuffer = SDL_CreateGPUBuffer(
			context->Device,
			&sceneVertexBufferCreateInfo
		);

		SDL_GPUBufferCreateInfo sceneIndexBufferCreateInfo {
			.usage = SDL_GPU_BUFFERUSAGE_INDEX,
			.size = sizeof(Uint16) * 36
		};
		SceneIndexBuffer = SDL_CreateGPUBuffer(
			context->Device,
			&sceneIndexBufferCreateInfo
		);

		SDL_GPUTransferBufferCreateInfo posColorTransferBufferCreateInfo {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = (sizeof(PositionColorVertex) * 24) + (sizeof(Uint16) * 36)
		};
		SDL_GPUTransferBuffer* bufferTransferBuffer = SDL_CreateGPUTransferBuffer(
			context->Device,
			&posColorTransferBufferCreateInfo
		);

		PositionColorVertex* transferData = static_cast<PositionColorVertex*>(SDL_MapGPUTransferBuffer(
			context->Device,
			bufferTransferBuffer,
			false
		));

		transferData[0] = PositionColorVertex { -10, -10, -10, 255, 0, 0, 255 };
		transferData[1] = PositionColorVertex { 10, -10, -10, 255, 0, 0, 255 };
		transferData[2] = PositionColorVertex { 10, 10, -10, 255, 0, 0, 255 };
		transferData[3] = PositionColorVertex { -10, 10, -10, 255, 0, 0, 255 };

		transferData[4] = PositionColorVertex { -10, -10, 10, 255, 255, 0, 255 };
		transferData[5] = PositionColorVertex { 10, -10, 10, 255, 255, 0, 255 };
		transferData[6] = PositionColorVertex { 10, 10, 10, 255, 255, 0, 255 };
		transferData[7] = PositionColorVertex { -10, 10, 10, 255, 255, 0, 255 };

		transferData[8] = PositionColorVertex { -10, -10, -10, 255, 0, 255, 255 };
		transferData[9] = PositionColorVertex { -10, 10, -10, 255, 0, 255, 255 };
		transferData[10] = PositionColorVertex { -10, 10, 10, 255, 0, 255, 255 };
		transferData[11] = PositionColorVertex { -10, -10, 10, 255, 0, 255, 255 };

		transferData[12] = PositionColorVertex { 10, -10, -10, 0, 255, 0, 255 };
		transferData[13] = PositionColorVertex { 10, 10, -10, 0, 255, 0, 255 };
		transferData[14] = PositionColorVertex { 10, 10, 10, 0, 255, 0, 255 };
		transferData[15] = PositionColorVertex { 10, -10, 10, 0, 255, 0, 255 };

		transferData[16] = PositionColorVertex { -10, -10, -10, 0, 255, 255, 255 };
		transferData[17] = PositionColorVertex { -10, -10, 10, 0, 255, 255, 255 };
		transferData[18] = PositionColorVertex { 10, -10, 10, 0, 255, 255, 255 };
		transferData[19] = PositionColorVertex { 10, -10, -10, 0, 255, 255, 255 };

		transferData[20] = PositionColorVertex { -10, 10, -10, 0, 0, 255, 255 };
		transferData[21] = PositionColorVertex { -10, 10, 10, 0, 0, 255, 255 };
		transferData[22] = PositionColorVertex { 10, 10, 10, 0, 0, 255, 255 };
		transferData[23] = PositionColorVertex { 10, 10, -10, 0, 0, 255, 255 };

		Uint16* indexData = (Uint16*) &transferData[24];
		Uint16 indices[] = {
			0, 1, 2, 0, 2, 3,
			4, 5, 6, 4, 6, 7,
			8, 9, 10, 8, 10, 11,
			12, 13, 14, 12, 14, 15,
			16, 17, 18, 16, 18, 19,
			20, 21, 22, 20, 22, 23
		};
		SDL_memcpy(indexData, indices, sizeof(indices));

		SDL_UnmapGPUTransferBuffer(context->Device, bufferTransferBuffer);

		// Upload the transfer data to the GPU buffers
		SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(context->Device);
		SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

		SDL_GPUTransferBufferLocation vertexTransferBufferLocation{
			.transfer_buffer = bufferTransferBuffer,
			.offset = 0
		};
		SDL_GPUBufferRegion vertexBufferRegion{
			.buffer = SceneVertexBuffer,
			.offset = 0,
			.size = sizeof(PositionColorVertex) * 24
		};
		SDL_UploadToGPUBuffer(
			copyPass,
			&vertexTransferBufferLocation,
			&vertexBufferRegion,
			false
		);

		SDL_GPUTransferBufferLocation indexTransferBufferLocation{
			.transfer_buffer = bufferTransferBuffer,
			.offset = sizeof(PositionColorVertex) * 24
	};
		SDL_GPUBufferRegion indexBufferRegion{
			.buffer = SceneIndexBuffer,
			.offset = 0,
			.size = sizeof(Uint16) * 36
	};
		SDL_UploadToGPUBuffer(
			copyPass,
			&indexTransferBufferLocation,
			&indexBufferRegion,
			false
		);

		SDL_EndGPUCopyPass(copyPass);
		SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
		SDL_ReleaseGPUTransferBuffer(context->Device, bufferTransferBuffer);
	}

	Time = 0;
	return 0;
}

static int Update_cpp(Context* context)
{
	Time += context->DeltaTime;
	return 0;
}

static int Draw_cpp(Context* context)
{
	SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(context->Device);
	if (cmdbuf == nullptr)
	{
		SDL_Log("AcquireGPUCommandBuffer failed: %s", SDL_GetError());
		return -1;
	}

	SDL_GPUTexture* swapchainTexture;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, context->Window, &swapchainTexture, nullptr, nullptr)) {
		SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
		return -1;
	}

	if (swapchainTexture != nullptr)
	{
		// Render the 3D Scene (Color and Depth pass)
		float nearPlane = 20.0f;
		float farPlane = 60.0f;

		Matrix4x4 proj = Matrix4x4_CreatePerspectiveFieldOfView(
			75.0f * SDL_PI_F / 180.0f,
			SceneWidth / (float)SceneHeight,
			nearPlane,
			farPlane
		);
		Matrix4x4 view = Matrix4x4_CreateLookAt(
			Vector3{ SDL_cosf(Time) * 30, 30, SDL_sinf(Time) * 30 },
			Vector3{ 0, 0, 0 },
			Vector3{ 0, 1, 0 }
		);

		Matrix4x4 viewproj = Matrix4x4_Multiply(view, proj);

		SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo = { 0 };
		depthStencilTargetInfo.texture = SceneDepthTexture;
		depthStencilTargetInfo.cycle = true;
		depthStencilTargetInfo.clear_depth = 1;
		depthStencilTargetInfo.clear_stencil = 0;
		depthStencilTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
		depthStencilTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
		depthStencilTargetInfo.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
		depthStencilTargetInfo.stencil_store_op = SDL_GPU_STOREOP_STORE;

		SDL_PushGPUVertexUniformData(cmdbuf, 0, &viewproj, sizeof(viewproj));
		std::array<float, 2> nearFarPlane{nearPlane, farPlane};
		SDL_PushGPUFragmentUniformData(cmdbuf, 0, nearFarPlane.data(), 8);

		// Render the object
		SDL_GPUColorTargetInfo swapchainTargetInfo = { 0 };
		swapchainTargetInfo.texture = swapchainTexture;
		swapchainTargetInfo.clear_color = SDL_FColor{ 0.2f, 0.5f, 0.4f, 1.0f };
		swapchainTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
		swapchainTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

		SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &swapchainTargetInfo, 1, &depthStencilTargetInfo);
		SDL_GPUBufferBinding sceneVertexBufferBinding{.buffer = SceneVertexBuffer, .offset = 0 };
		SDL_BindGPUVertexBuffers(renderPass, 0, &sceneVertexBufferBinding, 1);
		SDL_GPUBufferBinding sceneIndexBufferBinding{ .buffer = SceneIndexBuffer, .offset = 0 };
		SDL_BindGPUIndexBuffer(renderPass, &sceneIndexBufferBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);
		SDL_BindGPUGraphicsPipeline(renderPass, ScenePipeline);
		SDL_DrawGPUIndexedPrimitives(renderPass, 36, 1, 0, 0, 0);
		SDL_EndGPURenderPass(renderPass);
	}

	SDL_SubmitGPUCommandBuffer(cmdbuf);

	return 0;
}

static void Quit_cpp(Context* context)
{
	SDL_ReleaseGPUGraphicsPipeline(context->Device, ScenePipeline);
	SDL_ReleaseGPUTexture(context->Device, SceneDepthTexture);
	SDL_ReleaseGPUBuffer(context->Device, SceneVertexBuffer);
	SDL_ReleaseGPUBuffer(context->Device, SceneIndexBuffer);

	CommonQuit(context);
}


extern "C" {

static int Init(Context* context) { return Init_cpp(context); }
static int Update(Context* context) { return Update_cpp(context); }
static int Draw(Context* context) { return Draw_cpp(context); }
static void Quit(Context* context) { return Quit_cpp(context); }

Example GlTF_Example = { "glTF", Init, Update, Draw, Quit };

} // extern "C"