#include <array>
#include <numeric>

extern "C" {

#include "Common.h"

} // extern "C"

#include "glTFModel.h"

static SDL_GPUGraphicsPipeline* ScenePipeline;
static SDL_GPUTexture* SceneDepthTexture;

static float Time;
static int SceneWidth, SceneHeight;
static float WorldScale = 1.0f;

typedef struct GlmPositionColorVertex
{
	glm::vec3 pos;
	glm::vec4 color;
} GlmPositionColorVertex;

struct SmartContext_t {
	std::unique_ptr<vks::VulkanDevice> vulkanDevice{};
	std::unique_ptr<vkglTF::Model> model{};

	using IndexType = Uint32;
	static constexpr SDL_GPUIndexElementSize index_element_size = SDL_GPU_INDEXELEMENTSIZE_32BIT;


	Uint32 vertCount{};
	Uint32 indexCount{};

	vkglTF::BoundingBox aabb{};
};

static SmartContext_t* SmartContext = nullptr;

static int Init_cpp(Context* context)
{
	int result = CommonInit(context, 0);
	if (result < 0)
	{
		return result;
	}

	// Creates the Shaders & Pipelines
	{
		//SDL_GPUShader* sceneVertexShader = LoadShader(context->Device, "PositionColorTransform.vert", 0, 1, 0, 0);
		//if (sceneVertexShader == nullptr)
		//{
		//	SDL_Log("Failed to create 'PositionColorTransform' vertex shader!");
		//	return -1;
		//}

		//SDL_GPUShader* sceneFragmentShader = LoadShader(context->Device, "SolidColorDepth.frag", 0, 1, 0, 0);
		//if (sceneFragmentShader == nullptr)
		//{
		//	SDL_Log("Failed to create 'SolidColorDepth' fragment shader!");
		//	return -1;
		//}



		// Create the shaders
		SDL_GPUShader* sceneVertexShader = LoadShader(context->Device, "TexturedQuad.vert", 0, 0, 0, 0); // PRECHECKIN: needs a new shader, but the texture is loading
		if (sceneVertexShader == NULL)
		{
			SDL_Log("Failed to create vertex shader!");
			return -1;
		}

		SDL_GPUShader* sceneFragmentShader = LoadShader(context->Device, "TexturedQuad.frag", 1, 0, 0, 0);
		if (sceneFragmentShader == NULL)
		{
			SDL_Log("Failed to create fragment shader!");
			return -1;
		}




		std::array<SDL_GPUVertexBufferDescription, 1> sceneVertexBufferDescription{{{
			.slot = 0,
			.pitch = sizeof(GlmPositionColorVertex),
			.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
			.instance_step_rate = 0
		}}};
		std::array<SDL_GPUVertexAttribute, 2> sceneVertexAttribute{{
			{ // position
				.location = 0,
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
				.offset = 0
			},
			//{ // color
			//	.location = 1,
			//	.buffer_slot = 0,
			//	.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
			//	.offset = sizeof(float) * 3
			//}
			{ // UV
				.location = 1,
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
				.offset = sizeof(float) * 3
			}
		}};
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

	SmartContext = new SmartContext_t{};

	{

		SmartContext->vulkanDevice = std::make_unique<vks::VulkanDevice>(context->Device, context->Window);
		SmartContext->model = std::make_unique<vkglTF::Model>();
		//SmartContext->model->loadFromFile("Content/Models/DamagedHelmet/glTF-Embedded/DamagedHelmet.gltf", SmartContext->vulkanDevice.get(), 1.0f /*scale*/);
		//SmartContext->model->loadFromFile("Content/Models/Box/glTF-Embedded/Box.gltf", SmartContext->vulkanDevice.get(), 1.0f /*scale*/);
		//SmartContext->model->loadFromFile("Content/Models/green_cube/glTF/green_cube.gltf", SmartContext->vulkanDevice.get(), 1.0f /*scale*/);
		SmartContext->model->loadFromFile("Content/Models/Dumpy/glTF/dumpy.gltf", SmartContext->vulkanDevice.get(), 1.0f /*scale*/);



		SmartContext->vertCount = static_cast<Uint32>(SmartContext->model->vertexBufferHackyStorage.size());
		SmartContext->indexCount = static_cast<Uint32>(SmartContext->model->indexBufferHackyStorage.size());


		SmartContext->aabb = vkglTF::BoundingBox(glm::vec3(-1), glm::vec3(1));





		const std::array<float, 6> allSides{
			std::abs(SmartContext->aabb.min.x),
			std::abs(SmartContext->aabb.min.y),
			std::abs(SmartContext->aabb.min.z),
			std::abs(SmartContext->aabb.max.x),
			std::abs(SmartContext->aabb.max.y),
			std::abs(SmartContext->aabb.max.z),
		};

		const float longestSide = std::reduce(allSides.begin(), allSides.end(), 0.0f, [](float left, float right){ return std::max(left, right); });
		WorldScale = 10.0f / longestSide;
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

		Matrix4x4 world = Matrix4x4_CreateScale( // PRECHECKIN: cleanup
			WorldScale,
			WorldScale,
			WorldScale
		);
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

		//Matrix4x4 viewproj = Matrix4x4_Multiply(view, proj); // PRECHECKIN: cleanup
		Matrix4x4 viewproj = Matrix4x4_Multiply(Matrix4x4_Multiply(world, view), proj);

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
		SDL_BindGPUGraphicsPipeline(renderPass, ScenePipeline);

		SDL_GPUTextureSamplerBinding textureSamplerBinding{ .texture = SmartContext->model->textures[0].view, .sampler = SmartContext->model->textures[0].sampler };
		SDL_BindGPUFragmentSamplers(renderPass, 0, &textureSamplerBinding, 1);

		SmartContext->model->draw(renderPass);

		SDL_EndGPURenderPass(renderPass);
	}

	SDL_SubmitGPUCommandBuffer(cmdbuf);

	return 0;
}

static void Quit_cpp(Context* context)
{
	delete SmartContext;
	SmartContext = nullptr;

	SDL_ReleaseGPUGraphicsPipeline(context->Device, ScenePipeline);
	SDL_ReleaseGPUTexture(context->Device, SceneDepthTexture);

	CommonQuit(context);
}


extern "C" {

static int Init(Context* context) { return Init_cpp(context); }
static int Update(Context* context) { return Update_cpp(context); }
static int Draw(Context* context) { return Draw_cpp(context); }
static void Quit(Context* context) { return Quit_cpp(context); }

Example GlTF_Example = { "glTF", Init, Update, Draw, Quit };

} // extern "C"