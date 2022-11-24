#pragma once
#include "pch.h"

namespace boost {
	namespace serialization {
		class access;
	}
}
struct Renderer
{
	size_t meshID;
	size_t transformID;
	std::vector<size_t> materialIDs;

	Renderer(size_t meshID, size_t transformID, std::vector<size_t>&& materialIDs);
	Renderer(size_t meshID, size_t transformID, std::vector<size_t>& materialIDs);
	
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

	bool operator ==(const Renderer& other) const;

private:
	friend class boost::serialization::access;
	Renderer() : meshID(), transformID() { }
};