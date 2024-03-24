#include "Device.h"


// local callback functions
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,VkDebugUtilsMessageTypeFlagsEXT messageType,const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,const VkAllocationCallbacks* pAllocator,VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance,
        "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,VkDebugUtilsMessengerEXT debugMessenger,const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance,
        "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

Magnet::VKBase::Device::Device(Magnet::Window& window) : window{ window } {
    
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createCommandPool();
}

Magnet::VKBase::Device::~Device()
{
    vkDestroyCommandPool(device_, commandPool, nullptr);
    vkDestroyDevice(device_, nullptr);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface_, nullptr);
    vkDestroyInstance(instance, nullptr);
}

uint32_t Magnet::VKBase::Device::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

VkFormat Magnet::VKBase::Device::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (
            tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    throw std::runtime_error("failed to find supported format!");
}

void Magnet::VKBase::Device::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device_, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device_, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device_, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }

    vkBindBufferMemory(device_, buffer, bufferMemory, 0);
}

VkCommandBuffer Magnet::VKBase::Device::beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void Magnet::VKBase::Device::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue_);

    vkFreeCommandBuffers(device_, commandPool, 1, &commandBuffer);
}

void Magnet::VKBase::Device::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;  // Optional
    copyRegion.dstOffset = 0;  // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void Magnet::VKBase::Device::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layerCount;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);
    endSingleTimeCommands(commandBuffer);
}

void Magnet::VKBase::Device::createImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
    if (vkCreateImage(device_, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device_, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device_, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    if (vkBindImageMemory(device_, image, imageMemory, 0) != VK_SUCCESS) {
        throw std::runtime_error("failed to bind image memory!");
    }
}

void Magnet::VKBase::Device::createInstance()
{
    if (enableValidationLayers && !checkValidationLayersSupport()) {
        throw std::runtime_error("Validation layers requested, but not available");
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName ="Magnet";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;



    /// -------------------------------- ///
    ///       INSTANCE EXTENSIONS        ///
    /// -------------------------------- ///
    {
        uint32_t instanceExtensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(instanceExtensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, extensions.data());

        std::cout << "\nAvailable Instance Extensions :" << std::endl;
        std::cout << "------------------------------" << std::endl;
        std::unordered_set<std::string> available;
        for (const auto& extension : extensions) {
            std::cout << "\t" << extension.extensionName << std::endl;
            available.insert(extension.extensionName);
        }

        auto requiredinstanceExtensions = getInstanceRequiredExtensions();

        std::cout << "\nRequired Instance Extensions :" << std::endl;
        std::cout << "------------------------------" << std::endl;
        for (const auto& required : requiredinstanceExtensions) {
            std::cout << "\t" << required << std::endl;
            if (available.find(required) == available.end()) {
                throw std::runtime_error("Missing Required Extension");
            }
            else {
                usedInstanceExtensions.push_back(required);
            }
        }

        std::cout << "\nUsed Instance Extensions :" << std::endl;
        std::cout << "------------------------------" << std::endl;
        for (auto& used_Extension : usedInstanceExtensions) {
            std::cout << "\t" << used_Extension << std::endl;
        }
        std::cout << "\n";
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(usedInstanceExtensions.size());
    createInfo.ppEnabledExtensionNames = usedInstanceExtensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers) {

        uint32_t instanceLayerCount = 0;
        vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
        std::vector<VkLayerProperties> layers(instanceLayerCount);
        vkEnumerateInstanceLayerProperties(&instanceLayerCount, layers.data());

        std::cout << "\nAvailable Validation Layers :" << std::endl;
        std::cout << "------------------------------" << std::endl;
        std::unordered_set<std::string> available;
        for (const auto& layer : layers) {
            std::cout << "\t" << layer.layerName << std::endl;
            available.insert(layer.layerName);
        }

        auto requiredValidationLayers = getRequiredValidationLayers();

        std::cout << "\nRequired Validation Layers :" << std::endl;
        std::cout << "------------------------------" << std::endl;
        for (const auto& required : requiredValidationLayers) {
            std::cout << "\t" << required << std::endl;
            if (available.find(required) == available.end()) {
                throw std::runtime_error("Missing Required Validation Layer");
            }
            else {
                usedValidationLayers.push_back(required);
            }
        }

        std::cout << "\nUsed Validation Layers :" << std::endl;
        std::cout << "------------------------------" << std::endl;
        for (auto& used_Layer : usedValidationLayers) {
            std::cout << "\t" << used_Layer << std::endl;
        }
        std::cout << "\n";

        createInfo.enabledLayerCount = static_cast<uint32_t>(usedValidationLayers.size());
        createInfo.ppEnabledLayerNames = usedValidationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }

}

void Magnet::VKBase::Device::setupDebugMessenger()
{
    if (!enableValidationLayers) return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);
    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void Magnet::VKBase::Device::createSurface()
{
    window.createWindowSurface(instance, &surface_);
}

std::string getVendorName(uint32_t vendorID)
{
    switch (vendorID)
    {
    case 0x1002:
        return ("AMD");
    case 0x1010:
        return ("ImgTec");
    case 0x10DE:
        return ("NVIDIA");
    case 0x13B5:
        return ("ARM");
    case 0x5143:
        return ("Qualcomm");
    case 0x8086:
        return ("INTEL");
    }
    return ("Unknown Vendor");
}

std::string getDeviceType(uint32_t deviceType)
{
    switch (deviceType)
    {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
        return ("Other");
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return ("Integrated GPU");
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        return ("Discrete GPU");
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        return ("Virtual GPU");
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        return ("CPU");
    }
    return ("Unknown");
}

std::string getVersionString(uint32_t version)
{
    char str[64];
    sprintf(str, "%d.%d.%d", VK_VERSION_MAJOR(version), VK_VERSION_MINOR(version), VK_VERSION_PATCH(version));
    return std::string(str);
}

void Magnet::VKBase::Device::pickPhysicalDevice()
{
    std::cout << "\n------------------------------\nDevices : \n\n";

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }
    std::cout << "Devices count : " << deviceCount << std::endl;
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (int i = 0; i < devices.size(); i++) {
        vkGetPhysicalDeviceProperties(devices[i], &properties);
        std::cout << "Device " << i<<" : "<<std::endl;
        std::cout << "\t- Device Name : " << properties.deviceName << std::endl;
        std::cout << "\t- Vendor : " << getVendorName(properties.vendorID)<<std::endl;
        std::cout << "\t- Driver Version : "<< getVersionString(properties.driverVersion) <<std::endl;
        std::cout << "\t- API Version : "<< getVersionString(properties.driverVersion) << std::endl;
        std::cout << "\t- Device Type : "<< getDeviceType(properties.deviceType) << std::endl;
    }

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    std::cout << "Chosen device : \n" << std::endl;
    std::cout << "\t" << properties.deviceName<<"\n\n";
}

void Magnet::VKBase::Device::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(usedDeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = usedDeviceExtensions.data();

    // might not really be necessary anymore because device specific validation layers
    // have been deprecated
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(usedValidationLayers.size());
        createInfo.ppEnabledLayerNames = usedValidationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(device_, indices.graphicsFamily, 0, &graphicsQueue_);
    vkGetDeviceQueue(device_, indices.presentFamily, 0, &presentQueue_);
}

void Magnet::VKBase::Device::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
    poolInfo.flags =
        VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

bool Magnet::VKBase::Device::isDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

std::vector<const char*> Magnet::VKBase::Device::getInstanceRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

std::vector<const char*> Magnet::VKBase::Device::getRequiredValidationLayers()
{
    std::vector<const char*> requiredLayers;
    requiredLayers.push_back("VK_LAYER_KHRONOS_validation");

    return requiredLayers;
}

std::vector<const char*> Magnet::VKBase::Device::getDeviceRequiredExtensions()
{
    std::vector<const char*> requireddeviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    return requireddeviceExtensions;
}

bool Magnet::VKBase::Device::checkValidationLayersSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : usedValidationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

Magnet::VKBase::QueueFamilyIndices Magnet::VKBase::Device::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
            indices.graphicsFamilyHasValue = true;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);
        if (queueFamily.queueCount > 0 && presentSupport) {
            indices.presentFamily = i;
            indices.presentFamilyHasValue = true;
        }
        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

void Magnet::VKBase::Device::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;  // Optional
}

bool Magnet::VKBase::Device::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(device, &props);

    std::cout << "\nTesting Extensions Compatibility on device : " << props.deviceName<<std::endl;

    std::cout << "\nAvailable Device Extensions :" << std::endl;
    std::cout << "------------------------------" << std::endl;
    std::unordered_set<std::string> available;
    for (const auto& extension : availableExtensions) {
        std::cout << "\t" << extension.extensionName << std::endl;
        available.insert(extension.extensionName);
    }

    auto requireddeviceExtensions = getDeviceRequiredExtensions();

    std::cout << "\nRequired Device Extensions :" << std::endl;
    std::cout << "------------------------------" << std::endl;
    for (const auto& required : requireddeviceExtensions) {
        std::cout << "\t" << required << std::endl;
        if (available.find(required) == available.end()) {
            return false;
        }
        else {
            usedDeviceExtensions.push_back(required);
        }
    }
    std::cout << "\nUsed Device Extensions :" << std::endl;
    std::cout << "------------------------------" << std::endl;
    for (auto& used_Extension : usedDeviceExtensions) {
        std::cout << "\t" << used_Extension << std::endl;
    }
    std::cout << "\n";

    return true;

    
}

Magnet::VKBase::SwapChainSupportDetails Magnet::VKBase::Device::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device,
            surface_,
            &presentModeCount,
            details.presentModes.data());
    }
    return details;
}
