#pragma once
#include "Commons.h"
#include "Camera.h"

namespace Magnet {
	namespace VKBase{
		struct FrameInfo {
			int frameIndex;
			float frameTime;
			VkCommandBuffer commandBuffer;
			EngineBase::Camera& camera;
			VkDescriptorSet globalDescriptorSet;
		};
	}
}