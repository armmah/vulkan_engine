#include "pch.h"
#include "ProfileMarker.h"

std::chrono::steady_clock::time_point ProfileMarkerBase::getNow() noexcept { return std::chrono::steady_clock::now(); }
auto ProfileMarkerBase::getMiliseconds(Timestamp startTime, Timestamp endTime) noexcept { return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count(); }
auto ProfileMarkerBase::getMicroseconds(Timestamp startTime, Timestamp endTime) noexcept { return std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count(); }

ProfileMarkerBase::ProfileMarkerBase() : startTime(getNow()) { }

ProfileMarkerInjectResult::ProfileMarkerInjectResult(int64_t& toBeInjected) : ProfileMarkerBase(), resultToInject(&toBeInjected) { }
ProfileMarkerInjectResult::~ProfileMarkerInjectResult()
{
	*resultToInject = getMiliseconds(startTime, getNow());
}

ProfileMarker::ProfileMarker(std::string markerName) : ProfileMarkerBase(), markerName(markerName) { }
ProfileMarker::~ProfileMarker()
{
	auto msElapsed = getMiliseconds(startTime, getNow());
	printf("=\\ %s /=: {%lld ms}\n", markerName.c_str(), msElapsed);
}

ProfilerMarkerAccumulative::ProfilerMarkerAccumulative(std::string markerName)
	: markerName(markerName) { }

ProfilerMarkerAccumulative::~ProfilerMarkerAccumulative()
{
	auto msElapsed = getMiliseconds(startTime, getNow());
	accumulated[markerName] += msElapsed;
	printf("=\\ %s /=: total {%lld ms}, self {%lld ms}\n", markerName.c_str(), accumulated[markerName], msElapsed);
}
