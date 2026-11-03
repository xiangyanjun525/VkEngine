#pragma once
#include "Base.h"

namespace Engine
{
	struct Vertex_PosColor
	{
		glm::vec3 mPosition;
		glm::vec3 mColor;

		static std::vector<VkVertexInputBindingDescription> getVertexInputBindingDescriptions(VkVertexInputRate inputRate)
		{
			std::vector<VkVertexInputBindingDescription> bindingDes{};
			bindingDes.resize(1);
			bindingDes[0].binding = 0;
			bindingDes[0].inputRate = inputRate;
			bindingDes[0].stride = sizeof(Vertex_PosColor);

			return bindingDes;
		}
		static std::vector<VkVertexInputAttributeDescription> getVertexInputAttributeDescriptions()
		{
			std::vector<VkVertexInputAttributeDescription> attributeDes{};
			attributeDes.resize(2);
			attributeDes[0].binding = 0;
			attributeDes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDes[0].location = 0;
			attributeDes[0].offset = offsetof(Vertex_PosColor, mPosition);;

			attributeDes[1].binding = 0;
			attributeDes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDes[1].location = 1;
			attributeDes[1].offset = offsetof(Vertex_PosColor, mColor);

			return attributeDes;
		}
	};
}
