#include <iridium/iridium.hpp>
#include <iridium/air.hpp>
#include <iridium/spirv.hpp>
#include <iridium/dynamic-llvm.hpp>

#include <llvm-c/Core.h>

#include <cstdlib>

IRIDIUM_API bool Iridium::init() {
#ifndef _WIN32
	return Iridium::DynamicLLVM::init();
#else
	return true;
#endif
};

IRIDIUM_API void Iridium::finit() {
#ifndef _WIN32
	Iridium::DynamicLLVM::finit();
#else

#endif
};

IRIDIUM_API void* Iridium::translate(const void* inputData, size_t inputSize, size_t& outputSize, OutputInfo& outputInfo) {
	AIR::Library lib(inputData, inputSize);
	SPIRV::Builder builder;

	lib.buildModule(builder, outputInfo);

	auto result = builder.finalize(outputSize);

	return result;
};
