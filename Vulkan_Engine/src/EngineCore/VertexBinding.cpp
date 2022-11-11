#include "pch.h"
#include "CollectionUtility.h"
#include "VertexBinding.h"
#include "Mesh.h"

#include "VkTypes/InitializersUtility.h"

bool VertexBinding::validateAttributeAndBindingDescriptions(const std::vector<VkVertexInputBindingDescription>& bindingDescriptions, const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions)
{
	//1. Validate (vertexBindingDescriptionCount <= VkPhysicalDeviceLimits::maxVertexInputBindings)
	//2. Validate (vertexAttributeDescriptionCount <= VkPhysicalDeviceLimits::maxVertexInputAttributes)

	// 3. bindingDescription array should not have conflicting bindings (ensure unique)
	std::set<uint32_t> uniqueBindings;
	for (auto& bindDesc : bindingDescriptions)
	{
		if (uniqueBindings.count(bindDesc.binding))
		{
			printf("A conflicting binding was detected in the binding descriptions.");
			return false;
		}
		uniqueBindings.insert(bindDesc.binding);
	}

	// 4. Foreach attributeDescriptions[i].binding validate that bindingDescription with the same binding exists
	for (auto& attrDesc : attributeDescriptions)
	{
		if (uniqueBindings.count(attrDesc.binding) == 0)
		{
			printf("The required binding could not be found.");
			return false;
		}
	}

	// 5. attributeDescriptions array should not have conflicting locations (ensure unique)
	std::set<uint32_t> uniqueLocations;
	for (auto& attrDesc : attributeDescriptions)
	{
		if (uniqueLocations.count(attrDesc.location))
		{
			printf("A conflicting location was detected in the vertex attribute descriptions.");
			return false;
		}

		uniqueLocations.insert(attrDesc.location);
	}

	return true;
}
