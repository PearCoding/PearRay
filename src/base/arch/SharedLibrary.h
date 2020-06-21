#pragma once

#include "PR_Config.h"

namespace PR {
class PR_LIB_BASE SharedLibrary {
public:
	SharedLibrary();
	SharedLibrary(const std::wstring& file);
	~SharedLibrary();

	void* symbol(const std::string& name) const;
	void unload();

	inline operator bool() const { return mInternal != nullptr; }

private:
	std::shared_ptr<class SharedLibraryInternal> mInternal;
};
} // namespace PR