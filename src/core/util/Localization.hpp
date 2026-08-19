#pragma once
#include <string>

namespace YimMenu
{
	// Returns the Turkish translation of the given English string.
	// Returns the input string unchanged when no translation is available.
	std::string tr(const std::string& key);
}
