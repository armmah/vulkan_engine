#pragma once
#include "pch.h"

namespace Presentation
{
	class Pass
	{
	public:
		Pass(bool isEnabled);
		bool getActive() const;
		void setActive(bool isActive);

		virtual void release(VkDevice device) = 0;

	private:
		bool m_isEnabled;
	};
}