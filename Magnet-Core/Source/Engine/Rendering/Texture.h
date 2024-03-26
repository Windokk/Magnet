#pragma once
#include "../../Commons.h"

namespace Magnet {
	namespace EngineBase{
		namespace Rendering {
			class Texture
			{
			public:
				vks::VulkanDevice* device;
				VkImage               image;
				VkImageLayout         imageLayout;
				VkDeviceMemory        deviceMemory;
				VkImageView           view;
				uint32_t              width, height;
				uint32_t              mipLevels;
				uint32_t              layerCount;
				VkDescriptorImageInfo descriptor;
				VkSampler             sampler;

				void      updateDescriptor();
				void      destroy();
				ktxResult loadKTXFile(std::string filename, ktxTexture** target);
			};

			class Texture2D : public Texture
			{
			public:
				void loadFromFile(
					std::string        filename,
					VkFormat           format,
					vks::VulkanDevice* device,
					VkQueue            copyQueue,
					VkImageUsageFlags  imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
					VkImageLayout      imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					bool               forceLinear = false);
				void fromBuffer(
					void* buffer,
					VkDeviceSize       bufferSize,
					VkFormat           format,
					uint32_t           texWidth,
					uint32_t           texHeight,
					vks::VulkanDevice* device,
					VkQueue            copyQueue,
					VkFilter           filter = VK_FILTER_LINEAR,
					VkImageUsageFlags  imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
					VkImageLayout      imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			};

			class Texture2DArray : public Texture
			{
			public:
				void loadFromFile(
					std::string        filename,
					VkFormat           format,
					vks::VulkanDevice* device,
					VkQueue            copyQueue,
					VkImageUsageFlags  imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
					VkImageLayout      imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			};

			class TextureCubeMap : public Texture
			{
			public:
				void loadFromFile(
					std::string        filename,
					VkFormat           format,
					vks::VulkanDevice* device,
					VkQueue            copyQueue,
					VkImageUsageFlags  imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
					VkImageLayout      imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			};
		}
	}
}