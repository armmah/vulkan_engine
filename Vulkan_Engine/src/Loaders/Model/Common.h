#pragma once
#include "pch.h"

#include "VertexBinding.h"
#ifdef _DEBUG
#define IMPORT_LIMITER_ENABLED

// The limiter helps when debugging some things to iterate quickly
#ifdef IMPORT_LIMITER_ENABLED
constexpr int import_mesh_limitter = 0;
#endif
#endif

struct TextureSource;
typedef size_t MeshIndex;
typedef std::vector<TextureSource> SubmeshMaterials;

typedef size_t MaterialID;
typedef size_t IndexCount;

namespace tinyobj
{
	struct IndexHash
	{
		static constexpr size_t cantor(size_t a, size_t b);
		static constexpr size_t cantor(int a, int b, int c);

		size_t operator()(const tinyobj::index_t& k) const;
	};

	struct IndexComparison
	{
		bool operator()(const tinyobj::index_t& a, const tinyobj::index_t& b) const;
	};
}

BOOST_IS_BITWISE_SERIALIZABLE(glm::vec2)
BOOST_IS_BITWISE_SERIALIZABLE(glm::vec3)
BOOST_IS_BITWISE_SERIALIZABLE(glm::vec4)
BOOST_IS_BITWISE_SERIALIZABLE(glm::mat4)
namespace boost::serialization
{
	template <typename Ar>
	void serialize(Ar& ar, glm::vec2& v, unsigned _)
	{
		ar& v.x& v.y;
	}

	template <typename Ar>
	void serialize(Ar& ar, glm::vec3& v, unsigned _)
	{
		ar& v.x& v.y& v.z;
	}

	template <typename Ar>
	void serialize(Ar& ar, glm::vec4& v, unsigned _)
	{
		ar& v.x& v.y& v.z& v.w;
	}

	template <typename Ar>
	void serialize(Ar& ar, glm::mat4& m, unsigned _)
	{
		ar& m[0];
		ar& m[1];
		ar& m[2];
		ar& m[3];
	}
}

namespace Loader
{
	class Utility
	{
	public:
		template <typename T>
		static void fillArrayWithDefaultValue(std::vector<T>& dst, size_t offset, size_t count)
		{
			for (int i = offset; i < std::min(dst.size(), count); i++)
			{
				dst[i] = T();
			}
		}

		template <typename F, typename T>
		static void reinterpretCopy(std::vector<F>& src, std::vector<T>& dst)
		{
			F* r_src = (F*)src.data();
			reinterpretCopy(src.data(), src.size(), dst);
		}

		template <typename F, typename T>
		static void reinterpretCopy(const F* r_src, size_t count_src, std::vector<T>& dst)
		{
			T* r_dst = (T*)dst.data();

			auto byteSize_src = count_src * sizeof(F);
			auto byteSize_dst = dst.size() * sizeof(T);
			auto minSize = byteSize_dst < byteSize_src ? byteSize_dst : byteSize_src;

			// Copy the contents of src to dst until dst is filled.
			memcpy(r_dst, r_src, minSize);

			// Fill the rest, if necessary.
			//fillArrayWithDefaultValue(dst, minSize / sizeof(T), dst.size());
			for (size_t i = minSize / sizeof(T); i < dst.size(); i++)
			{
				dst[i] = T();
			}
		}

		template <typename F, typename T>
		static T reinterpretAt(std::vector<F>& src, size_t srcIndex)
		{
			srcIndex *= (sizeof(T) / sizeof(F));
			return *reinterpret_cast<T*>(&src[srcIndex]);
		}

		template <typename F, typename T>
		static T reinterpretAt_orFallback(std::vector<F>& src, size_t srcIndex)
		{
			return src.size() > srcIndex ? reinterpretAt<F, T>(src, srcIndex) : T{};
		}

		static void minVector(glm::vec3& min, const glm::vec3& point);
		static void maxVector(glm::vec3& max, const glm::vec3& point);

		static void assertIndex(int index, int vertCount, const char* name);
	};
}