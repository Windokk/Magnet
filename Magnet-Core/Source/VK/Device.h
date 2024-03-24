#pragma once
#include "../Commons.h"
#include "../Window.h"

namespace Magnet {

    namespace VKBase {

        struct PhysicalDeviceInfo
        {
            VkPhysicalDeviceMemoryProperties     memoryProperties{};
            std::vector<VkQueueFamilyProperties> queueProperties;

            VkPhysicalDeviceFeatures         features10{};
            VkPhysicalDeviceVulkan11Features features11{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
            VkPhysicalDeviceVulkan12Features features12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
            VkPhysicalDeviceVulkan13Features features13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };

            VkPhysicalDeviceProperties         properties10{};
            VkPhysicalDeviceVulkan11Properties properties11{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES };
            VkPhysicalDeviceVulkan12Properties properties12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES };
            VkPhysicalDeviceVulkan13Properties properties13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES };
        };

        struct SwapChainSupportDetails {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

        struct QueueFamilyIndices {
            uint32_t graphicsFamily;
            uint32_t presentFamily;
            bool graphicsFamilyHasValue = false;
            bool presentFamilyHasValue = false;
            bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
        };

        class Device {
        public:
#ifdef NDEBUG
            const bool enableValidationLayers = false;
#else
            const bool enableValidationLayers = true;
#endif

            Device(Magnet::Window& window);
            ~Device();

            // Not copyable or movable
            Device(const Device&) = delete;
            Device& operator=(const Device&) = delete;
            Device(Device&&) = delete;
            Device& operator=(Device&&) = delete;

            VkCommandPool getCommandPool() { return commandPool; }
            VkDevice device() { return device_; }
            VkSurfaceKHR surface() { return surface_; }
            VkQueue graphicsQueue() { return graphicsQueue_; }
            VkQueue presentQueue() { return presentQueue_; }

            SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
            uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
            QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }
            VkFormat findSupportedFormat(
                const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

            // Buffer Helper Functions
            void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
            VkCommandBuffer beginSingleTimeCommands();
            void endSingleTimeCommands(VkCommandBuffer commandBuffer);
            void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
            void copyBufferToImage(
                VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

            void createImageWithInfo(
                const VkImageCreateInfo& imageInfo,
                VkMemoryPropertyFlags properties,
                VkImage& image,
                VkDeviceMemory& imageMemory);

            VkPhysicalDeviceProperties properties;

        private:
            void createInstance();
            void setupDebugMessenger();
            void createSurface();
            void pickPhysicalDevice();
            void createLogicalDevice();
            void createCommandPool();

            // helper functions
            bool isDeviceSuitable(VkPhysicalDevice device);
            std::vector<const char*> getInstanceRequiredExtensions();
            std::vector<const char*> getRequiredValidationLayers();
            std::vector<const char*> getDeviceRequiredExtensions();
            bool checkValidationLayersSupport();
            QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
            void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
            bool checkDeviceExtensionSupport(VkPhysicalDevice device);
            SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

            VkInstance instance;
            VkDebugUtilsMessengerEXT debugMessenger;
            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
            Magnet::Window& window;
            VkCommandPool commandPool;

            VkDevice device_;
            VkSurfaceKHR surface_;
            VkQueue graphicsQueue_;
            VkQueue presentQueue_;


            VkPhysicalDeviceAccelerationStructureFeaturesKHR accelFeature{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR };
            VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtPipelineFeature{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR };
                                                                             
            PhysicalDeviceInfo physicalInfo;

            std::vector<const char*> usedValidationLayers;
            std::vector<const char*> usedInstanceExtensions;
            std::vector<const char*> usedDeviceExtensions;
        };

    }
}