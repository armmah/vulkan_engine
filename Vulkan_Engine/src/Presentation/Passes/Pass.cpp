#include "pch.h"
#include "Pass.h"

Presentation::Pass::Pass(bool isEnabled) : m_isEnabled(isEnabled) { }

bool Presentation::Pass::getActive() const { return m_isEnabled; }

void Presentation::Pass::setActive(bool isActive) { m_isEnabled = isActive; }
