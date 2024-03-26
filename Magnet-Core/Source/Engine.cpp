#include "Engine.h"



Magnet::Engine::Engine()
{
    init();
}

Magnet::Engine::~Engine()
{
    vkDestroyPipeline(device.device(), pipelines.solid, nullptr);
    if (pipelines.wireframe != VK_NULL_HANDLE) {
        vkDestroyPipeline(device.device(), pipelines.wireframe, nullptr);
    }
    vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device.device(), descriptorSetLayouts.matrices, nullptr);
    vkDestroyDescriptorSetLayout(device.device(), descriptorSetLayouts.textures, nullptr);
    shaderData.buffer.destroy();
}

void Magnet::Engine::init()
{

    camera.type = EngineBase::Camera::CameraType::lookat;
    camera.flipY = true;
    camera.setPosition(glm::vec3(0.0f, -0.1f, -1.0f));
    camera.setRotation(glm::vec3(0.0f, 45.0f, 0.0f));
    camera.setPerspective(60.0f, (float)WIDTH / (float)HEIGHT, 0.1f, 256.0f);

    //loadObjects();
    /*
    initSwapchain();
    createCommandPool();
    setupSwapChain();
    createCommandBuffers();
    createSynchronizationPrimitives();
    setupDepthStencil();
    setupRenderPass();
    createPipelineCache();
    setupFrameBuffer();
    ////////////////////////
    loadAssets();
    prepareUniformBuffers();
    setupDescriptors();
    preparePipelines();
    buildCommandBuffers();
    */


    renderer.createPipelineCache();
    swapchain.createFramebuffers();
    loadAssets();
    prepareUniformBuffers();
    setupDescriptors();
    preparePipelines();
    buildCommandBuffers();
    

}

void Magnet::Engine::run() {

    glfwPollEvents();
}

void Magnet::Engine::waitIdle()
{
    vkDeviceWaitIdle(device.device());
    glfwDestroyWindow(window.getGLFWWindow());
    glfwTerminate();
}

bool Magnet::Engine::shouldClose()
{
    return window.shouldClose();
}

void Magnet::Engine::loadglTFFile(std::string filename) {
	tinygltf::Model glTFInput;
	tinygltf::TinyGLTF gltfContext;
	std::string error, warning;

	bool fileLoaded = gltfContext.LoadASCIIFromFile(&glTFInput, &error, &warning, filename);

	// Pass some Vulkan resources required for setup and rendering to the glTF model loading class
	glTFModel.vulkanDevice = &device;
	glTFModel.copyQueue = queue;

	std::vector<uint32_t> indexBuffer;
	std::vector<VulkanglTFModel::Vertex> vertexBuffer;
	if (fileLoaded) {
		glTFModel.loadImages(glTFInput);
		glTFModel.loadMaterials(glTFInput);
		glTFModel.loadTextures(glTFInput);
		const tinygltf::Scene& scene = glTFInput.scenes[0];
		for (size_t i = 0; i < scene.nodes.size(); i++) {
				const tinygltf::Node node = glTFInput.nodes[scene.nodes[i]];
				glTFModel.loadNode(node, glTFInput, nullptr, indexBuffer, vertexBuffer);
			}
		}
		else {
			vks::tools::exitFatal("Could not open the glTF file.\n\nMake sure the assets submodule has been checked out and is up-to-date.", -1);
			return;
		}

		// Create and upload vertex and index buffer
		// We will be using one single vertex buffer and one single index buffer for the whole glTF scene
		// Primitives (of the glTF model) will then index into these using index offsets

		size_t vertexBufferSize = vertexBuffer.size() * sizeof(VulkanglTFModel::Vertex);
		size_t indexBufferSize = indexBuffer.size() * sizeof(uint32_t);
		glTFModel.indices.count = static_cast<uint32_t>(indexBuffer.size());

		struct StagingBuffer {
			VkBuffer buffer;
			VkDeviceMemory memory;
		} vertexStaging, indexStaging;

		// Create host visible staging buffers (source)
		VK_CHECK_RESULT(vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			vertexBufferSize,
			&vertexStaging.buffer,
			&vertexStaging.memory,
			vertexBuffer.data()));
		// Index data
		VK_CHECK_RESULT(vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			indexBufferSize,
			&indexStaging.buffer,
			&indexStaging.memory,
			indexBuffer.data()));

		// Create device local buffers (target)
		VK_CHECK_RESULT(vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vertexBufferSize,
			&glTFModel.vertices.buffer,
			&glTFModel.vertices.memory));
		VK_CHECK_RESULT(vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			indexBufferSize,
			&glTFModel.indices.buffer,
			&glTFModel.indices.memory));

		// Copy data from staging buffers (host) do device local buffer (gpu)
		VkCommandBuffer copyCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		VkBufferCopy copyRegion = {};

		copyRegion.size = vertexBufferSize;
		vkCmdCopyBuffer(
			copyCmd,
			vertexStaging.buffer,
			glTFModel.vertices.buffer,
			1,
			&copyRegion);

		copyRegion.size = indexBufferSize;
		vkCmdCopyBuffer(
			copyCmd,
			indexStaging.buffer,
			glTFModel.indices.buffer,
			1,
			&copyRegion);

		vulkanDevice->flushCommandBuffer(copyCmd, queue, true);

		// Free staging resources
		vkDestroyBuffer(device, vertexStaging.buffer, nullptr);
		vkFreeMemory(device, vertexStaging.memory, nullptr);
		vkDestroyBuffer(device, indexStaging.buffer, nullptr);
		vkFreeMemory(device, indexStaging.memory, nullptr);
}

void Magnet::Engine::loadAssets()
{
    loadglTFFile("");
}