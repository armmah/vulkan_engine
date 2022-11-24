#pragma once

template<typename T>
static T bitFlagAppend(T state, T flag)
{
	return static_cast<T>(state | 1 << flag);
}

template<typename T>
static T bitFlagAppendIf(T state, bool condition, T flag)
{
	return static_cast<T>(state | (condition << flag));
}

template<typename T>
static bool bitFlagPresent(T state, T flag)
{
	return (state & (1 << flag)) != 0;
}