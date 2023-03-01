#pragma once

#include <memory>

#include <llvm-c/Core.h>
#include <iridium/dynamic-llvm.hpp>

namespace Iridium {
	namespace LLVMSupport {
#ifndef _WIN32
		using Module = std::unique_ptr<LLVMOpaqueModule, std::reference_wrapper<DynamicLLVM::DynamicFunction<decltype(LLVMDisposeModule)>>>;
		using MemoryBuffer = std::unique_ptr<LLVMOpaqueMemoryBuffer, std::reference_wrapper<DynamicLLVM::DynamicFunction<decltype(LLVMDisposeMemoryBuffer)>>>;
#else
		using Module = std::unique_ptr<LLVMOpaqueModule, std::reference_wrapper<decltype(LLVMDisposeModule)>>;
		using MemoryBuffer = std::unique_ptr<LLVMOpaqueMemoryBuffer, std::reference_wrapper<decltype(LLVMDisposeMemoryBuffer)>>;
#endif
	};
};
