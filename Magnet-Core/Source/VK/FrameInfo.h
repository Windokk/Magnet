#pragma once
#include "../Commons.h"
#include "../Engine/Camera.h"

namespace Magnet {
	namespace VKBase{
		struct FrameInfo {
			int frameIndex;
			float frameTime;
			VkCommandBuffer commandBuffer;
			EngineBase::Camera& camera;
			VkDescriptorSet globalDescriptorSet;
			EngineBase::Object::Map& objects;
		};
	}
}