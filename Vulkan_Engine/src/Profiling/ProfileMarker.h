#pragma once
#include "pch.h"

struct ProfileMarkerBase
{
	typedef std::chrono::steady_clock::time_point Timestamp;

	Timestamp startTime;

	static auto getNow() noexcept;
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