#pragma once

#include <llvm-c/BitReader.h>
#include <llvm-c/Core.h>
#ifndef _WIN32
#include <dlfcn.h>
#endif
#include <mutex>

namespace Iridium {
	namespace DynamicLLVM {
		extern void* libraryHandle;

		bool init();
		void finit();
#ifndef _WIN32
		template<typename FuncPtr>
		struct DynamicFunction;

		template<typename Ret, typename... Arg>
		struct DynamicFunction<Ret(Arg...)> {
			Ret (*pointer)(Arg...);
			std::once_flag _resolveFlag;
			const char* name;

			DynamicFunction(const char* _name):
				name(_name)
				{};

			bool resolve() {
				std::call_once(_resolveFlag, [&]() {
					pointer = reinterpret_cast<decltype(pointer)>(dlsym(libraryHandle, name));
				});
				return !!pointer;
			};

			bool isAvailable() {
				return resolve();
			};

			operator bool() {
				return isAvailable();
			};

			Ret operator ()(Arg... arg) {
				if (!resolve()) {
					throw std::runtime_error(std::string("Failed to resolve ") + name);
				}
				return pointer(std::forward<Arg>(arg)...);
			};
		};

		#define IRIDIUM_DYNAMICLLVM_FUNCTION_FOREACH(_macro) \
			_macro(LLVMBasicBlockAsValue) \
			_macro(LLVMConstIntGetSExtValue) \
			_macro(LLVMConstIntGetZExtValue) \
			_macro(LLVMConstRealGetDouble) \
			_macro(LLVMCountIncoming) \
			_macro(LLVMCountParamTypes) \
			_macro(LLVMCountStructElementTypes) \
			_macro(LLVMCreateMemoryBufferWithMemoryRange) \
			_macro(LLVMDisposeMemoryBuffer) \
			_macro(LLVMDisposeMessage) \
			_macro(LLVMDisposeModule) \
			_macro(LLVMGetAlignment) \
			_macro(LLVMGetArrayLength) \
			_macro(LLVMGetCalledValue) \
			_macro(LLVMGetCondition) \
			_macro(LLVMGetConstOpcode) \
			_macro(LLVMGetElementAsConstant) \
			_macro(LLVMGetElementType) \
			_macro(LLVMGetFCmpPredicate) \
			_macro(LLVMGetFirstBasicBlock) \
			_macro(LLVMGetFirstInstruction) \
			_macro(LLVMGetFirstNamedMetadata) \
			_macro(LLVMGetIncomingBlock) \
			_macro(LLVMGetIncomingValue) \
			_macro(LLVMGetIndices) \
			_macro(LLVMGetInitializer) \
			_macro(LLVMGetInstructionOpcode) \
			_macro(LLVMGetIntTypeWidth) \
			_macro(LLVMGetLastInstruction) \
			_macro(LLVMGetMaskValue) \
			_macro(LLVMGetMDNodeNumOperands) \
			_macro(LLVMGetMDNodeOperands) \
			_macro(LLVMGetMDString) \
			_macro(LLVMGetNamedFunction) \
			_macro(LLVMGetNamedMetadata) \
			_macro(LLVMGetNamedMetadataNumOperands) \
			_macro(LLVMGetNamedMetadataOperands) \
			_macro(LLVMGetNextBasicBlock) \
			_macro(LLVMGetNextInstruction) \
			_macro(LLVMGetNumContainedTypes) \
			_macro(LLVMGetNumIndices) \
			_macro(LLVMGetNumMaskElements) \
			_macro(LLVMGetNumOperands) \
			_macro(LLVMGetOperand) \
			_macro(LLVMGetParam) \
			_macro(LLVMGetParamTypes) \
			_macro(LLVMGetPointerAddressSpace) \
			_macro(LLVMGetReturnType) \
			_macro(LLVMGetSuccessor) \
			_macro(LLVMGetTypeKind) \
			_macro(LLVMGetUndefMaskElem) \
			_macro(LLVMGetValueKind) \
			_macro(LLVMGetValueName2) \
			_macro(LLVMGetVectorSize) \
			_macro(LLVMIsAMDString) \
			_macro(LLVMIsConditional) \
			_macro(LLVMIsPackedStruct) \
			_macro(LLVMParseBitcode2) \
			_macro(LLVMPrintModuleToString) \
			_macro(LLVMStructGetTypeAtIndex) \
			_macro(LLVMTypeOf)

		#define IRIDIUM_DYNAMICLLVM_FUNCTION_DECL(_name) \
			extern DynamicFunction<decltype(_name)> _name;

		IRIDIUM_DYNAMICLLVM_FUNCTION_FOREACH(IRIDIUM_DYNAMICLLVM_FUNCTION_DECL)
#endif
	};
};
