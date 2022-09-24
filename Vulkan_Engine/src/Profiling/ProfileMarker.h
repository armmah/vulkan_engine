#pragma once
#include "pch.h"

struct ProfileMarkerBase
{
	typedef std::chrono::steady_clock::time_point Timestamp;

	Timestamp startTime;

	static std::chrono::steady_clock::time_point getNow() noexcept;
	static auto getMiliseconds(Timestamp startTime, Timestamp endTime) noexcept;
	static auto getMicroseconds(Timestamp startTime, Timestamp endTime) noexcept;

	ProfileMarkerBase();
};

struct ProfileMarkerInjectResult : ProfileMarkerBase
{
	int64_t* resultToInject;

	ProfileMarkerInjectResult(int64_t& toBeInjected);
	~ProfileMarkerInjectResult();
};

struct ProfileMarker : ProfileMarkerBase
{
	std::string markerName;

	ProfileMarker(std::string markerName);
	~ProfileMarker();
};

struct ProfilerMarkerAccumulative : ProfileMarkerBase
{
	std::string markerName;
	inline static std::unordered_map<std::string, long long> accumulated;

	ProfilerMarkerAccumulative(std::string markerName);
	~ProfilerMarkerAccumulative();
};