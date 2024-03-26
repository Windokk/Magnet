#pragma once
#include "../Commons.h"
#include "../Window.h"

namespace Magnet {

    namespace VKBase {


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

            VkPhysicalDeviceProperties properties;

            void createImageWithInfo(
                const VkImageCreateInfo& imageInfo,
                VkMemoryPropertyFlags properties,
                VkImage& image,
                VkDeviceMemory& imageMemory);

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

                                             

            std::vector<const char*> usedValidationLayers;
            std::vector<const char*> usedInstanceExtensions;
            std::vector<const char*> usedDeviceExtensions;
        };

    }
}