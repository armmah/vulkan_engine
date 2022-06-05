#pragma once

#include "Common.h"

#include "glm/gtc/quaternion.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>

#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"

#include "VertexBinding.h"
#include "VkMesh.h"
#include "Mesh.h"

class Scene
{
public:
	const std::vector<UNQ<Mesh>>& getMeshes() const { return meshes; }
	const std::vector<UNQ<VkMesh>>& getGraphicsMeshes() const { return graphicsMeshes; }
	const VertexBinding& getVertexBinding() const { return *vertexBinding; }

	bool load(const VmaAllocator& vmaAllocator)
	{
		meshes.resize(2);
		meshes[0] = MAKEUNQ<Mesh>(Mesh::getPrimitiveTriangle());
		meshes[1] = MAKEUNQ<Mesh>(Mesh::getPrimitiveCube());

		const auto count = meshes.size();
		graphicsMeshes.resize(count);
		Mesh::MeshDescriptor md;
		for(int i = 0; i < count; i++)
		{
			if (i == 0)
				md = meshes[i]->getMeshDescriptor();

			if (!meshes[i]->allocateGraphicsMesh(graphicsMeshes[i], vmaAllocator) || !meshes[i]->isValid())
				return false;

			if (md != meshes[i]->getMeshDescriptor())
			{
				printf("Mesh metadata does not match - can not bind to the same pipeline.");
				return false;
			}
		}
		
		Mesh::initializeBindings(vertexBinding, md);

		return vertexBinding->runValidations();
	}

	void release(const VmaAllocator& allocator)
	{
		for (auto& mesh : meshes)
		{
			mesh->clear();
			mesh.release();
		}

		for (auto& gmesh : graphicsMeshes)
		{
			gmesh->release(allocator);
		}

		vertexBinding.release();
	}

private:
	std::vector<UNQ<Mesh>> meshes;
	std::vector<UNQ<VkMesh>> graphicsMeshes;
	UNQ<VertexBinding> vertexBinding;
};
 