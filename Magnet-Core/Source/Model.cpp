#include "Model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyObjLoader/tiny_obj_loader.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "Utils.h"

namespace std{
	template<>
	struct hash<Magnet::VKBase::Model::Vertex> {
		size_t operator()(Magnet::VKBase::Model::Vertex const& vertex) const {
			size_t seed = 0;
			Magnet::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
			return seed;
		}
	};
}

Magnet::VKBase::Model::Model(Device& device, const Builder& builder) : device{device}
{
	createVertexBuffers(builder.vertices);
	createIndexBuffers(builder.indices);
}

Magnet::VKBase::Model::~Model()
{
	vkDestroyBuffer(device.device(), vertexBuffer, nullptr);
	vkFreeMemory(device.device(), vertexBufferMemory, nullptr);

	if (hasIndexBuffer) {
		vkDestroyBuffer(device.device(), indexBuffer, nullptr);
		vkFreeMemory(device.device(), indexBufferMemory, nullptr);
	}
}

void Magnet::VKBase::Model::createVertexBuffers(const std::vector<Vertex>& vertices)
{ 
	vertex_Count = static_cast<uint32_t>(vertices.size()); 
	assert(vertex_Count >= 3 && "Vertex count must be at least 3");

	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertex_Count;

	VkBuffer staggingBuffer;
	VkDeviceMemory staggingBufferMemory;

	device.createBuffer(
		bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		staggingBuffer,
		staggingBufferMemory);
	void* data;
	vkMapMemory(device.device(), staggingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(device.device(), staggingBufferMemory);

	device.createBuffer(
		bufferSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertexBuffer,
		vertexBufferMemory);

	device.copyBuffer(staggingBuffer, vertexBuffer, bufferSize);

	vkDestroyBuffer(device.device(), staggingBuffer, nullptr);
	vkFreeMemory(device.device(), staggingBufferMemory, nullptr);

}

void Magnet::VKBase::Model::createIndexBuffers(const std::vector<uint32_t>& indices)
{
	index_Count = static_cast<uint32_t>(indices.size());
	hasIndexBuffer = index_Count > 0;
	if (!hasIndexBuffer) {
		return;
	}
	VkDeviceSize bufferSize = sizeof(indices[0]) * index_Count;
	

	VkBuffer staggingBuffer;
	VkDeviceMemory staggingBufferMemory;

	device.createBuffer(
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		staggingBuffer,
		staggingBufferMemory);
	void* data;
	vkMapMemory(device.device(), staggingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(device.device(), staggingBufferMemory);

	device.createBuffer(
		bufferSize,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		indexBuffer,
		indexBufferMemory);

	device.copyBuffer(staggingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(device.device(), staggingBuffer, nullptr);
	vkFreeMemory(device.device(), staggingBufferMemory, nullptr);
}

void Magnet::VKBase::Model::draw(VkCommandBuffer commandBuffer)
{
	if (hasIndexBuffer) {
		vkCmdDrawIndexed(commandBuffer, index_Count, 1, 0, 0, 0);
	}
	else {
		vkCmdDraw(commandBuffer, vertex_Count, 1, 0, 0);
	}
}

std::unique_ptr<Magnet::VKBase::Model> Magnet::VKBase::Model::createModelFromFile(Device& device, const std::string& filepath)
{
	Builder builder{};
	builder.loadModel(filepath);

	return std::make_unique<Model>(device, builder);
}

void Magnet::VKBase::Model::bind(VkCommandBuffer commandBuffer)
{
	VkBuffer buffers[] = { vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

	if (hasIndexBuffer) {
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	}
}

std::vector<VkVertexInputBindingDescription> Magnet::VKBase::Model::Vertex::getBindingDescriptions()
{
	std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(Vertex);
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Magnet::VKBase::Model::Vertex::getAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

	attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) });
	attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });
	attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
	attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });

	return attributeDescriptions;
}

void Magnet::VKBase::Model::Builder::loadModel(const std::string& filepath)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
		throw std::runtime_error(warn + err);
	}

	vertices.clear();
	indices.clear();

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};
	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex{};

			if (index.vertex_index >= 0) {
				vertex.position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2],
				};

				vertex.color = {
					attrib.colors[3 * index.vertex_index + 0],
					attrib.colors[3 * index.vertex_index + 1],
					attrib.colors[3 * index.vertex_index + 2],
				};
			}

			if (index.normal_index >= 0) {
				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2],
				};
			}

			if (index.texcoord_index >= 0) {
				vertex.uv = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					attrib.texcoords[2 * index.texcoord_index + 1],
				};
			}

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}
			indices.push_back(uniqueVertices[vertex]);
		}
	}
}
