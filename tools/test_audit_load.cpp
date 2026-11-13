#include <dlfcn.h>
#include <link.h>

#include <cstdio>

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::fprintf(stderr, "usage: %s /path/to/SLSsteam.so\n", argv[0]);
		return 2;
	}

	void* handle = dlopen(argv[1], RTLD_NOW | RTLD_LOCAL);
	if (!handle)
	{
		std::fprintf(stderr, "dlopen failed: %s\n", dlerror());
		return 1;
	}

	dlerror();
	using LaVersion = unsigned int (*)(unsigned int);
	auto laVersion = reinterpret_cast<LaVersion>(dlsym(handle, "la_version"));
	const char* error = dlerror();
	if (error || !laVersion)
	{
		std::fprintf(stderr, "la_version lookup failed: %s\n",
		             error ? error : "symbol is null");
		dlclose(handle);
		return 1;
	}
	if (laVersion(LAV_CURRENT) != LAV_CURRENT)
	{
		std::fprintf(stderr, "unexpected audit ABI version\n");
		dlclose(handle);
		return 1;
	}

	std::puts("SLSsteam.so loaded in the i386 runtime and exposes the audit ABI");
	dlclose(handle);
	return 0;
}
