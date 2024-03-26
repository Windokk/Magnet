#pragma once
#include "Engine/Object.h"
#include "VK/Descriptors.h"
#include "Engine/Camera.h"
#include "Engine/Object.h"
#include "VK/Swapchain.h"
#include "Renderer.h"
#include "Engine/Rendering/Texture.h"

#include <tinygltf/tiny_gltf.h>

namespace Magnet {

	class VulkanglTFModel
	{
	public:
		// The class requires some Vulkan objects so it can create it's own resources
		Magnet::VKBase::Device* device;
		VkQueue copyQueue;

		// The vertex layout for the samples' model
		struct Vertex {
			glm::vec3 pos;
			glm::vec3 normal;
			glm::vec2 uv;
			glm::vec3 color;
		};

		// Single vertex buffer for all primitives
		struct {
			VkBuffer buffer;
			VkDeviceMemory memory;
		} vertices;

		// Single index buffer for all primitives
		struct {
			int count;
			VkBuffer buffer;
			VkDeviceMemory memory;
		} indices;

		// The following structures roughly represent the glTF scene structure
		// To keep things simple, they only contain those properties that are required for this sample
		struct Node;

		// A primitive contains the data for a single draw call
		struct Primitive {
			uint32_t firstIndex;
			uint32_t indexCount;
			int32_t materialIndex;
		};

		// Contains the node's (optional) geometry and can be made up of an arbitrary number of primitives
		struct Mesh {
			std::vector<Primitive> primitives;
		};

		// A node represents an object in the glTF scene graph
		struct Node {
			Node* parent;
			std::vector<Node*> children;
			Mesh mesh;
			glm::mat4 matrix;
			~Node() {
				for (auto& child : children) {
					delete child;
				}
			}
		};

		// A glTF material stores information in e.g. the texture that is attached to it and colors
		struct Material {
			glm::vec4 baseColorFactor = glm::vec4(1.0f);
			uint32_t baseColorTextureIndex;
		};

		// Contains the texture for a single glTF image
		// Images may be reused by texture objects and are as such separated
		struct Image {
			Magnet::EngineBase::Rendering::Texture2D texture;
			// We also store (and create) a descriptor set that's used to access this texture from the fragment shader
			VkDescriptorSet descriptorSet;
		};

		// A glTF texture stores a reference to the image and a sampler
		// In this sample, we are only interested in the image
		struct Texture {
			int32_t imageIndex;
		};

		/*
			Model data
		*/
		std::vector<Image> images;
		std::vector<Texture> textures;
		std::vector<Material> materials;
		std::vector<Node*> nodes;

		~VulkanglTFModel()
		{
			for (auto node : nodes) {
				delete node;
			}
			// Release all Vulkan resources allocated for the model
			vkDestroyBuffer(device->device(), vertices.buffer, nullptr);
			vkFreeMemory(device->device(), vertices.memory, nullptr);
			vkDestroyBuffer(device->device(), indices.buffer, nullptr);
			vkFreeMemory(device->device(), indices.memory, nullptr);
			for (Image image : images) {
				vkDestroyImageView(device->device(), image.texture.view, nullptr);
				vkDestroyImage(device->device(), image.texture.image, nullptr);
				vkDestroySampler(device->device(), image.texture.sampler, nullptr);
				vkFreeMemory(device->device(), image.texture.deviceMemory, nullptr);
			}
		}

		/*
			glTF loading functions

			The following functions take a glTF input model loaded via tinyglTF and convert all required data into our own structure
		*/

		void loadImages(tinygltf::Model& input)
		{
			// Images can be stored inside the glTF (which is the case for the sample model), so instead of directly
			// loading them from disk, we fetch them from the glTF loader and upload the buffers
			images.resize(input.images.size());
			for (size_t i = 0; i < input.images.size(); i++) {
				tinygltf::Image& glTFImage = input.images[i];
				// Get the image data from the glTF loader
				unsigned char* buffer = nullptr;
				VkDeviceSize bufferSize = 0;
				bool deleteBuffer = false;
				// We convert RGB-only images to RGBA, as most devices don't support RGB-formats in Vulkan
				if (glTFImage.component == 3) {
					bufferSize = glTFImage.width * glTFImage.height * 4;
					buffer = new unsigned char[bufferSize];
					unsigned char* rgba = buffer;
					unsigned char* rgb = &glTFImage.image[0];
					for (size_t i = 0; i < glTFImage.width * glTFImage.height; ++i) {
						memcpy(rgba, rgb, sizeof(unsigned char) * 3);
						rgba += 4;
						rgb += 3;
					}
					deleteBuffer = true;
				}
				else {
					buffer = &glTFImage.image[0];
					bufferSize = glTFImage.image.size();
				}
				// Load texture from image buffer
				images[i].texture.fromBuffer(buffer, bufferSize, VK_FORMAT_R8G8B8A8_UNORM, glTFImage.width, glTFImage.height, vulkanDevice, copyQueue);
				if (deleteBuffer) {
					delete[] buffer;
				}
			}
		}

		void loadTextures(tinygltf::Model& input)
		{
			textures.resize(input.textures.size());
			for (size_t i = 0; i < input.textures.size(); i++) {
				textures[i].imageIndex = input.textures[i].source;
			}
		}

		void loadMaterials(tinygltf::Model& input)
		{
			materials.resize(input.materials.size());
			for (size_t i = 0; i < input.materials.size(); i++) {
				// We only read the most basic properties required for our sample
				tinygltf::Material glTFMaterial = input.materials[i];
				// Get the base color factor
				if (glTFMaterial.values.find("baseColorFactor") != glTFMaterial.values.end()) {
					materials[i].baseColorFactor = glm::make_vec4(glTFMaterial.values["baseColorFactor"].ColorFactor().data());
				}
				// Get base color texture index
				if (glTFMaterial.values.find("baseColorTexture") != glTFMaterial.values.end()) {
					materials[i].baseColorTextureIndex = glTFMaterial.values["baseColorTexture"].TextureIndex();
				}
			}
		}

		void loadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, VulkanglTFModel::Node* parent, std::vector<uint32_t>& indexBuffer, std::vector<VulkanglTFModel::Vertex>& vertexBuffer)
		{
			VulkanglTFModel::Node* node = new VulkanglTFModel::Node{};
			node->matrix = glm::mat4(1.0f);
			node->parent = parent;

			// Get the local node matrix
			// It's either made up from translation, rotation, scale or a 4x4 matrix
			if (inputNode.translation.size() == 3) {
				node->matrix = glm::translate(node->matrix, glm::vec3(glm::make_vec3(inputNode.translation.data())));
			}
			if (inputNode.rotation.size() == 4) {
				glm::quat q = glm::make_quat(inputNode.rotation.data());
				node->matrix *= glm::mat4(q);
			}
			if (inputNode.scale.size() == 3) {
				node->matrix = glm::scale(node->matrix, glm::vec3(glm::make_vec3(inputNode.scale.data())));
			}
			if (inputNode.matrix.size() == 16) {
				node->matrix = glm::make_mat4x4(inputNode.matrix.data());
			};

			// Load node's children
			if (inputNode.children.size() > 0) {
				for (size_t i = 0; i < inputNode.children.size(); i++) {
					loadNode(input.nodes[inputNode.children[i]], input, node, indexBuffer, vertexBuffer);
				}
			}

			// If the node contains mesh data, we load vertices and indices from the buffers
			// In glTF this is done via accessors and buffer views
			if (inputNode.mesh > -1) {
				const tinygltf::Mesh mesh = input.meshes[inputNode.mesh];
				// Iterate through all primitives of this node's mesh
				for (size_t i = 0; i < mesh.primitives.size(); i++) {
					const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];
					uint32_t firstIndex = static_cast<uint32_t>(indexBuffer.size());
					uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer.size());
					uint32_t indexCount = 0;
					// Vertices
					{
						const float* positionBuffer = nullptr;
						const float* normalsBuffer = nullptr;
						const float* texCoordsBuffer = nullptr;
						size_t vertexCount = 0;

						// Get buffer data for vertex positions
						if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end()) {
							const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("POSITION")->second];
							const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
							positionBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
							vertexCount = accessor.count;
						}
						// Get buffer data for vertex normals
						if (glTFPrimitive.attributes.find("NORMAL") != glTFPrimitive.attributes.end()) {
							const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("NORMAL")->second];
							const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
							normalsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
						}
						// Get buffer data for vertex texture coordinates
						// glTF supports multiple sets, we only load the first one
						if (glTFPrimitive.attributes.find("TEXCOORD_0") != glTFPrimitive.attributes.end()) {
							const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("TEXCOORD_0")->second];
							const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
							texCoordsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
						}

						// Append data to model's vertex buffer
						for (size_t v = 0; v < vertexCount; v++) {
							Vertex vert{};
							vert.pos = glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
							vert.normal = glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
							vert.uv = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec3(0.0f);
							vert.color = glm::vec3(1.0f);
							vertexBuffer.push_back(vert);
						}
					}
					// Indices
					{
						const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.indices];
						const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
						const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];

						indexCount += static_cast<uint32_t>(accessor.count);

						// glTF supports different component types of indices
						switch (accessor.componentType) {
						case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
							const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
							for (size_t index = 0; index < accessor.count; index++) {
								indexBuffer.push_back(buf[index] + vertexStart);
							}
							break;
						}
						case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
							const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
							for (size_t index = 0; index < accessor.count; index++) {
								indexBuffer.push_back(buf[index] + vertexStart);
							}
							break;
						}
						case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
							const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
							for (size_t index = 0; index < accessor.count; index++) {
								indexBuffer.push_back(buf[index] + vertexStart);
							}
							break;
						}
						default:
							std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
							return;
						}
					}
					Primitive primitive{};
					primitive.firstIndex = firstIndex;
					primitive.indexCount = indexCount;
					primitive.materialIndex = glTFPrimitive.material;
					node->mesh.primitives.push_back(primitive);
				}
			}

			if (parent) {
				parent->children.push_back(node);
			}
			else {
				nodes.push_back(node);
			}
		}

		/*
			glTF rendering functions
		*/

		// Draw a single node including child nodes (if present)
		void drawNode(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VulkanglTFModel::Node* node)
		{
			if (node->mesh.primitives.size() > 0) {
				// Pass the node's matrix via push constants
				// Traverse the node hierarchy to the top-most parent to get the final matrix of the current node
				glm::mat4 nodeMatrix = node->matrix;
				VulkanglTFModel::Node* currentParent = node->parent;
				while (currentParent) {
					nodeMatrix = currentParent->matrix * nodeMatrix;
					currentParent = currentParent->parent;
				}
				// Pass the final matrix to the vertex shader using push constants
				vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &nodeMatrix);
				for (VulkanglTFModel::Primitive& primitive : node->mesh.primitives) {
					if (primitive.indexCount > 0) {
						// Get the texture index for this primitive
						VulkanglTFModel::Texture texture = textures[materials[primitive.materialIndex].baseColorTextureIndex];
						// Bind the descriptor for the current primitive's texture
						vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &images[texture.imageIndex].descriptorSet, 0, nullptr);
						vkCmdDrawIndexed(commandBuffer, primitive.indexCount, 1, primitive.firstIndex, 0, 0);
					}
				}
			}
			for (auto& child : node->children) {
				drawNode(commandBuffer, pipelineLayout, child);
			}
		}

		// Draw the glTF scene starting at the top-level-nodes
		void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
		{
			// All vertices and indices are stored in single buffers, so we only need to bind once
			VkDeviceSize offsets[1] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertices.buffer, offsets);
			vkCmdBindIndexBuffer(commandBuffer, indices.buffer, 0, VK_INDEX_TYPE_UINT32);
			// Render all nodes at top-level
			for (auto& node : nodes) {
				drawNode(commandBuffer, pipelineLayout, node);
			}
		}

	};

	struct GlobalUbo {
		glm::mat4 projection{ 1.f };
		glm::mat4 view{ 1.f };
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f };  // w is intensity
		glm::vec3 lightPosition{ -1.f };
		alignas(16) glm::vec4 lightColor{ 1.f };  // w is light intensity
	};

	

	class Engine {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;
		Engine();
		~Engine();

		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;

		void init();
		void run();
		void waitIdle();

		void loadAssets();

		bool shouldClose();

		void loadglTFFile(std::string filename);

		

	private:
		VulkanglTFModel glTFModel;

		Window window{ WIDTH, HEIGHT, "Magnet" };
		Magnet::VKBase::Device device{window};

		Magnet::EngineBase::Camera camera{};

		Magnet::VKBase::SwapChain swapchain{device, window.getExtent()};

		Renderer renderer{ window, device };

		EngineBase::Object::Map objects;

	};
}
