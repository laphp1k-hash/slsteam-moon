// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstddef>
#include <cstdint>
#include <dlfcn.h>
#include <string>

namespace Sha1Runtime
{
using Function = unsigned char* (*)(const unsigned char*, std::size_t,
                                    unsigned char*);

// Separated from symbol lookup so the failure contract can be tested without
// relying on the OpenSSL version installed on the test host.
inline bool computeWithFunction(const void* data, std::size_t size,
                                std::uint8_t out[20], Function function,
                                std::string& error)
{
	error.clear();
	if (!out)
	{
		error = "SHA-1 output buffer is null";
		return false;
	}
	if (!data && size != 0)
	{
		error = "SHA-1 input buffer is null";
		return false;
	}
	if (!function)
	{
		error = "OpenSSL SHA1 symbol is unavailable";
		return false;
	}
	if (!function(static_cast<const unsigned char*>(data), size, out))
	{
		error = "OpenSSL SHA1 computation failed";
		return false;
	}
	return true;
}

inline Function resolve()
{
	static Function function = []() -> Function {
		// Handles intentionally remain open because the cached function pointer
		// is used for the lifetime of the plugin.
		static void* handles[5]{};
		static const char* names[] = {"libcrypto.so.1.0.0", "libcrypto.so.1.1",
		                              "libcrypto.so.3", "libcrypto.so", nullptr};
		for (std::size_t i = 0; names[i]; ++i)
		{
			handles[i] = dlopen(names[i], RTLD_NOLOAD | RTLD_LAZY);
			if (!handles[i]) handles[i] = dlopen(names[i], RTLD_LAZY | RTLD_LOCAL);
			if (handles[i])
				if (auto symbol = reinterpret_cast<Function>(dlsym(handles[i], "SHA1")))
					return symbol;
		}
		return reinterpret_cast<Function>(dlsym(RTLD_DEFAULT, "SHA1"));
	}();
	return function;
}

inline bool compute(const void* data, std::size_t size, std::uint8_t out[20],
                    std::string& error)
{
	return computeWithFunction(data, size, out, resolve(), error);
}
} // namespace Sha1Runtime
