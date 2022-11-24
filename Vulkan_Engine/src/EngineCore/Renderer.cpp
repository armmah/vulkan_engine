#include "pch.h"
#include "Renderer.h"

Renderer::Renderer(size_t meshID, size_t transformID, std::vector<size_t>&& materialIDs)
	: meshID(meshID), transformID(transformID), materialIDs(materialIDs) { }

Renderer::Renderer(size_t meshID, size_t transformID, std::vector<size_t>& materialIDs)
	: meshID(meshID), transformID(transformID), materialIDs(std::move(materialIDs)) { }

bool Renderer::operator ==(const Renderer& other) const
{
	if (meshID != other.meshID || materialIDs.size() != other.materialIDs.size())
		return false;

	for (size_t i = 0; i < materialIDs.size(); i++)
	{
		if (materialIDs[i] != other.materialIDs[i])
			return false;
	}
	return true;
}