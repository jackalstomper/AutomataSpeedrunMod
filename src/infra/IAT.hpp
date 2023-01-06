#pragma once

#include "infra/defs.hpp"
#include <Windows.h>
#include <cstdint>

namespace IAT {

class IATHook {
	PIMAGE_THUNK_DATA m_thunkIAT;
	ULONGLONG m_originalFunction;

	void parseImports(u64 baseAddress, const char *moduleName, const char *functionName, LPCVOID replacementFunction);

	void readImportDescriptor(
			IMAGE_IMPORT_DESCRIPTOR &importDescriptor, u64 baseAddress, const char *functionName, LPCVOID replacementFunction
	);

public:
	IATHook(const char *moduleName, const char *functionName, LPCVOID replacementFunction);
	~IATHook();
};

} // namespace IAT
