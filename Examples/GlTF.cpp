#include <array>
#include <numeric>
#include <memory>

#if defined(_DEBUG) && defined(SDL_GLTF_DX_DEBUGGING)
#include <Initguid.h>
#include <dxgidebug.h> // For DXGIGetDebugInterface1 and IDXGIDebug1
#include <dxgi1_3.h>

#undef LoadImage
#endif

extern "C" {

#include "Common.h"
#include <SDL3/SDL_gpu.h>

// workaround for Debian build
#ifndef SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_STENCIL_NUMBER
#define SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_STENCIL_NUMBER SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_STENCIL_UINT8
#endif

} // extern "C"

#include "glTFModel.h"

static SDL_GPUGraphicsPipeline* ScenePipeline;
static SDL_GPUTexture* SceneDepthTexture;

static float Time;
static int SceneWidth, SceneHeight;
static float WorldScale = 1.0f;

static float ClearDepth = 1.0f;
static Uint8 ClearDepthStencil = 0;
static SDL_FColor ClearColor{ 0.2f, 0.5f, 0.4f, 1.0f };

//typedef struct GlmPositionColorVertex
//{
//	glm::vec3 pos;
//	glm::vec4 color;
//} GlmPositionColorVertex;

struct SmartContext_t {
	std::unique_ptr<vks::VulkanDevice> vulkanDevice{};
	std::unique_ptr<vkglTF::Model> model{};

	using IndexType = Uint32;
	static constexpr SDL_GPUIndexElementSize index_element_size = SDL_GPU_INDEXELEMENTSIZE_32BIT;


	//Uint32 vertCount{}; // PRECHECKIN: cleanup
	//Uint32 indexCount{};

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



		SDL_GPUShader* sceneVertexShader = LoadShader(context->Device, "PositionTexturedTransform.vert", 0, 2, 0, 0); // PRECHECKIN: needs a new shader, but the texture is loading
		if (sceneVertexShader == NULL)
		{
			SDL_Log("Failed to create vertex shader!");
			return -1;
		}

		int samplerCount = 0;
		++samplerCount; // colors
		++samplerCount; // normals

		SDL_GPUShader* sceneFragmentShader = LoadShader(context->Device, "TexturedDepth.frag", samplerCount, 1, 0, 0);
		if (sceneFragmentShader == NULL)
		{
			SDL_Log("Failed to create fragment shader!");
			return -1;
		}




		std::array<SDL_GPUVertexBufferDescription, 1> sceneVertexBufferDescription{{{
			.slot = 0,
			.pitch = sizeof(vkglTF::Model::Vertex),
			.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
			.instance_step_rate = 0
		}}};
		std::array<SDL_GPUVertexAttribute, 5> sceneVertexAttribute{{
			{ // position
				.location = 0,
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, // glm::vec3
				.offset = 0
			},
			{ // normal
				.location = 1,
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, // glm::vec3
				.offset = sizeof(glm::vec3)
			},
			//{ // color
			//	.location = 1,
			//	.buffer_slot = 0,
			//	.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, // glm::vec4
			//	.offset = sizeof(float) * 3
			//}
			{ // UV
				.location = 2,
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, // glm::vec2
				.offset = sizeof(glm::vec3) + sizeof(glm::vec3)
			},
			{ // joint
				.location = 3,
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_UINT4, // glm::uvec4
				.offset = sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2)
			},
			{ // weight
				.location = 4,
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, // glm::vec4
				.offset = sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::uvec4)
			}
		}};
		std::array<SDL_GPUColorTargetDescription, 1> sceneColorTargetDescription{{{
			.format = SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM // PRECHECKIN DXGI_FORMAT_B8G8R8A8_UNORM? was SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, read from swapchain tex?
		}}};
		SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {
			.vertex_shader = sceneVertexShader,
			.fragment_shader = sceneFragmentShader,
			.vertex_input_state = SDL_GPUVertexInputState{
				.vertex_buffer_descriptions = sceneVertexBufferDescription.data(),
				.num_vertex_buffers = 1,
				.vertex_attributes = sceneVertexAttribute.data(),
				.num_vertex_attributes = static_cast<Uint32>(sceneVertexAttribute.size())
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

		SDL_PropertiesID depthTexturePropertySet = SDL_CreateProperties();
		SDL_SetFloatProperty(depthTexturePropertySet, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_DEPTH_FLOAT, ClearDepth);
		SDL_SetFloatProperty(depthTexturePropertySet, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_STENCIL_NUMBER, ClearDepthStencil);

		SDL_GPUTextureCreateInfo depthTextureCreateInfo {
			.type = SDL_GPU_TEXTURETYPE_2D,
			.format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
			.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
			.width = static_cast<Uint32>(SceneWidth),
			.height = static_cast<Uint32>(SceneHeight),
			.layer_count_or_depth = 1,
			.num_levels = 1,
			.sample_count = SDL_GPU_SAMPLECOUNT_1,
			.props = depthTexturePropertySet
		};
		SceneDepthTexture = SDL_CreateGPUTexture(
			context->Device,
			&depthTextureCreateInfo
		);
	}

	// PRECHECKIN: address
	//D3D12 WARNING: ID3D12CommandList::ClearDepthStencilView: The clear values do not match those passed to resource creation. The clear operation is typically slower as a result; but will still clear to the desired value. [ EXECUTION WARNING #821: CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE]

	SmartContext = new SmartContext_t{};

	{
		SmartContext->vulkanDevice = std::make_unique<vks::VulkanDevice>(context->Device, context->Window);
		SmartContext->model = std::make_unique<vkglTF::Model>();
		//SmartContext->model->loadFromFile("Content/Models/DamagedHelmet/glTF-Embedded/DamagedHelmet.gltf", SmartContext->vulkanDevice.get(), 1.0f /*scale*/);
		//SmartContext->model->loadFromFile("Content/Models/Box/glTF-Embedded/Box.gltf", SmartContext->vulkanDevice.get(), 1.0f /*scale*/);
		//SmartContext->model->loadFromFile("Content/Models/green_cube/glTF/green_cube.gltf", SmartContext->vulkanDevice.get(), 1.0f /*scale*/);
		SmartContext->model->loadFromFile("Content/Models/Dumpy_normals/glTF/dumpy_norm.gltf", SmartContext->vulkanDevice.get(), 1.0f /*scale*/);

		// PRECHECKIN: this one is broken currently due to the many assumptions about model structure
		//SmartContext->model->loadFromFile("Content/Models/Dumpy_bread/glTF/dump_bread_float.gltf", SmartContext->vulkanDevice.get(), 1.0f /*scale*/);



		//SmartContext->vertCount = static_cast<Uint32>(SmartContext->model->vertexBufferHackyStorage.size()); // PRECHECKIN: cleanup
		//SmartContext->indexCount = static_cast<Uint32>(SmartContext->model->indexBufferHackyStorage.size());


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

//void updateMeshDataBuffer(uint32_t index)
//{
//	// @todo: optimize (no push, use fixed size)
//	std::vector<ShaderMeshData> shaderMeshData{};
//	for (auto& node : models.scene.linearNodes) {
//		ShaderMeshData meshData{};
//		if (node->mesh) {
//			memcpy(meshData.jointMatrix, node->mesh->jointMatrix, sizeof(glm::mat4) * MAX_NUM_JOINTS);
//			meshData.jointcount = node->mesh->jointcount;
//			meshData.matrix = node->mesh->matrix;
//			shaderMeshData.push_back(meshData);
//		}
//	}
//
//	VkDeviceSize bufferSize = shaderMeshData.size() * sizeof(ShaderMeshData);
//
//	if (!vulkanDevice->requiresStaging) {
//		memcpy(shaderMeshDataBuffers[index].mapped, shaderMeshData.data(), bufferSize);
//	}
//	else {
//		Buffer stagingBuffer;
//		VK_CHECK_RESULT(vulkanDevice->createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferSize, &stagingBuffer.buffer, &stagingBuffer.memory, shaderMeshData.data()));
//		// Copy from staging buffers
//		VkCommandBuffer copyCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
//		VkBufferCopy copyRegion{};
//		copyRegion.size = bufferSize;
//		vkCmdCopyBuffer(copyCmd, stagingBuffer.buffer, shaderMeshDataBuffers[index].buffer, 1, &copyRegion);
//		vulkanDevice->flushCommandBuffer(copyCmd, queue, true);
//		stagingBuffer.device = device;
//		stagingBuffer.destroy();
//	}
//}

static int Update_cpp(Context* context)
{
	static int32_t animationIndex = 0; // PRECHECKIN: hack
	static float animationTimer = 0.0f;


	Time += context->DeltaTime;

	if (/*(animate) &&*/ (SmartContext->model->animations.size() > 0)) {
		animationTimer += context->DeltaTime;
		if (animationTimer > SmartContext->model->animations[animationIndex].end) {
			animationTimer -= SmartContext->model->animations[animationIndex].end;
		}

		SmartContext->model->updateAnimation(animationIndex, animationTimer);
		//updateMeshDataBuffer(frameIndex);
	}

	return 0;
}

//__alignas_is_defined

struct alignas(16) VertexUBO {
	alignas(16) Matrix4x4 model{};
	alignas(16) Matrix4x4 view{};
	alignas(16) Matrix4x4 projection{};
};

static_assert(alignof(VertexUBO) == 16, "alignof(VertexUBO) should be 16");

struct alignas(16) MeshShaderDataBlock {
	alignas(16) glm::mat4 matrix;
	alignas(16) glm::mat4 jointMatrix[MAX_NUM_JOINTS]{};
	alignas(16) uint32_t jointcount = 0;
};


//typedef struct MeshShaderDataBlock_t {
//	glm::mat4 matrix;
//	union {
//		uint32_t jointcount = 0;
//		glm::vec4 pad;
//	};
//	glm::mat4 jointMatrix[MAX_NUM_JOINTS]{};
//} SDL_ALIGNED(16) MeshShaderDataBlock;

static_assert(alignof(MeshShaderDataBlock) == 16, "alignof(VertexUBO) should be 16");

struct alignas(16) MeshData {
	alignas(16) MeshShaderDataBlock meshData;
};

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

		VertexUBO vertexUBO{};

		vertexUBO.model = Matrix4x4_CreateScale( // PRECHECKIN: cleanup
			WorldScale,
			WorldScale,
			WorldScale
		);

		//Matrix4x4 rotateModel = Matrix4x4_CreateLookAt(
		//	Vector3{ SDL_cosf(Time) * 30, 30, SDL_sinf(Time) * 30 },
		//	Vector3{ 0, 0, 0 },
		//	Vector3{ 0, 0, 0 }
		//);

		//vertexUBO.model = Matrix4x4_Multiply(Matrix4x4_Multiply(vertexUBO.model, rotateModel), vertexUBO.projection);

		vertexUBO.projection = Matrix4x4_CreatePerspectiveFieldOfView(
			75.0f * SDL_PI_F / 180.0f,
			SceneWidth / (float)SceneHeight,
			nearPlane,
			farPlane
		);

#define ROTATE_CAMERA

//#ifdef ROTATE_CAMERA
		vertexUBO.view = Matrix4x4_CreateLookAt(
			Vector3{ SDL_cosf(Time) * 30, 30, SDL_sinf(Time) * 30 },
			Vector3{ 0, 0, 0 },
			Vector3{ 0, 1, 0 }
		);
//#else
//		const float fixedCameraAngleTime = 1.0f;
//		vertexUBO.view = Matrix4x4_CreateLookAt(
//			Vector3{ SDL_cosf(fixedCameraAngleTime) * 30, 30, SDL_sinf(fixedCameraAngleTime) * 30 },
//			Vector3{ 0, 0, 0 },
//			Vector3{ 0, 1, 0 }
//		);
//#endif

		//Matrix4x4 viewproj = Matrix4x4_Multiply(view, proj); // PRECHECKIN: cleanup
		//Matrix4x4 viewproj = Matrix4x4_Multiply(Matrix4x4_Multiply(world, view), proj);
		//vertexUBO.xfrm = Matrix4x4_Multiply(Matrix4x4_Multiply(vertexUBO.world, vertexUBO.view), vertexUBO.projection);



		vkglTF::Node* curNode = SmartContext->model->nodes.at(0)->children.at(0); // PRECHECKIN: how to get the real one?

		MeshData meshData{};
		meshData.meshData.matrix = curNode->mesh->matrix;
		meshData.meshData.jointcount = curNode->mesh->jointcount;
		SDL_memcpy(meshData.meshData.jointMatrix, curNode->mesh->jointMatrix, sizeof(curNode->mesh->jointMatrix));
		static_assert(sizeof(meshData.meshData.jointMatrix) == sizeof(curNode->mesh->jointMatrix));


		SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo = { 0 };
		depthStencilTargetInfo.texture = SceneDepthTexture;
		depthStencilTargetInfo.cycle = true;
		depthStencilTargetInfo.clear_depth = ClearDepth;
		depthStencilTargetInfo.clear_stencil = ClearDepthStencil;
		depthStencilTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
		depthStencilTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
		depthStencilTargetInfo.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
		depthStencilTargetInfo.stencil_store_op = SDL_GPU_STOREOP_STORE;

		SDL_PushGPUVertexUniformData(cmdbuf, 0, &vertexUBO, sizeof(vertexUBO));
		SDL_PushGPUVertexUniformData(cmdbuf, 1, &meshData, sizeof(meshData));

		std::array<float, 2> nearFarPlane{nearPlane, farPlane};
		SDL_PushGPUFragmentUniformData(cmdbuf, 0, nearFarPlane.data(), 8);

		// Render the object
		SDL_GPUColorTargetInfo swapchainTargetInfo = { 0 };
		swapchainTargetInfo.texture = swapchainTexture;
		swapchainTargetInfo.clear_color = ClearColor;
		swapchainTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
		swapchainTargetInfo.store_op = SDL_GPU_STOREOP_STORE;





		SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &swapchainTargetInfo, 1, &depthStencilTargetInfo);
		SDL_BindGPUGraphicsPipeline(renderPass, ScenePipeline);


		// NOTE: this is hacky and making assumptions about the model, look at https://github.com/SaschaWillems/Vulkan-glTF-PBR, VulkanApplication::renderNode for the proper way
		for (const vkglTF::Material& material : SmartContext->model->materials) {
			// look for the first material with a baseColorTexture
			if (material.baseColorTexture == nullptr) {
				continue;
			}

			enum class TextureIndex : size_t {
				Colors = 0,
				Normals = 1
			};

			std::array<SDL_GPUTextureSamplerBinding, 2> textureSamplerBindings{};
			textureSamplerBindings.at(static_cast<size_t>(TextureIndex::Colors)) = SDL_GPUTextureSamplerBinding{ .texture = material.baseColorTexture->view, .sampler = material.baseColorTexture->sampler };
			textureSamplerBindings.at(static_cast<size_t>(TextureIndex::Normals)) = SDL_GPUTextureSamplerBinding{ .texture = material.normalTexture->view, .sampler = material.normalTexture->sampler };

			SDL_BindGPUFragmentSamplers(renderPass, 0, textureSamplerBindings.data(), static_cast<Uint32>(textureSamplerBindings.size()));

			SmartContext->model->draw(renderPass);

			// hacky, rendering everything with the first material which has a baseColorTexture
			break;
		}

		SDL_EndGPURenderPass(renderPass);
	}

	SDL_SubmitGPUCommandBuffer(cmdbuf);

	return 0;
}

static void Quit_cpp(Context* context)
{
	SmartContext->model->destroy(context->Device);

	delete SmartContext;
	SmartContext = nullptr;

	SDL_ReleaseGPUGraphicsPipeline(context->Device, ScenePipeline);
	SDL_ReleaseGPUTexture(context->Device, SceneDepthTexture);

	CommonQuit(context);

#if defined(_DEBUG) && defined(SDL_GLTF_DX_DEBUGGING)
	IDXGIDebug1* pDebug = nullptr;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
	{
		pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, (DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		pDebug->Release();
	}
#endif
}


extern "C" {

static int Init(Context* context) { return Init_cpp(context); }
static int Update(Context* context) { return Update_cpp(context); }
static int Draw(Context* context) { return Draw_cpp(context); }
static void Quit(Context* context) { return Quit_cpp(context); }

Example GlTF_Example = { "glTF", Init, Update, Draw, Quit };

} // extern "C"