#include "../src/utils/sha1_runtime.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <string>

int main()
{
	const char input[] = "abc";
	std::array<std::uint8_t, 20> digest{};
	digest.fill(0xA5);
	std::string error;

	// Model dlopen/dlsym failure. The helper must report it and leave the
	// caller's buffer untouched, rather than manufacturing a zero hash.
	assert(!Sha1Runtime::computeWithFunction(
		input, sizeof(input) - 1, digest.data(), nullptr, error));
	assert(error == "OpenSSL SHA1 symbol is unavailable");
	assert(std::all_of(digest.begin(), digest.end(),
		[](std::uint8_t byte) { return byte == 0xA5; }));

	// Exercise the real runtime symbol resolver against the standard SHA-1
	// known-answer vector for "abc".
	assert(Sha1Runtime::compute(input, sizeof(input) - 1, digest.data(), error));
	const std::array<std::uint8_t, 20> expected = {
		0xa9, 0x99, 0x3e, 0x36, 0x47, 0x06, 0x81, 0x6a, 0xba, 0x3e,
		0x25, 0x71, 0x78, 0x50, 0xc2, 0x6c, 0x9c, 0xd0, 0xd8, 0x9d};
	assert(digest == expected);
	std::puts("SHA-1 resolution, failure contract, and known-answer test passed");
	return 0;
}
