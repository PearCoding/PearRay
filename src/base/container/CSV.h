#pragma once

#include "PR_Config.h"
#include <filesystem>
#include <iostream>

namespace PR {

/// Reads/Writes fixed column sized character/colon separated value file
/// Our implementation only allows floating point entries
class PR_LIB_BASE CSV {
public:
	CSV(size_t rows, size_t cols);
	explicit CSV(const std::filesystem::path& path);
	explicit CSV(std::istream& stream);

	inline size_t rowCount() const { return mRowCount; }
	inline size_t columnCount() const { return mColumnCount; }

	inline float& operator()(size_t i, size_t j) { return mData[i * mColumnCount + j]; }
	inline float operator()(size_t i, size_t j) const { return mData[i * mColumnCount + j]; }

	inline std::string& header(size_t c) { return mHeader[c]; }
	inline std::string header(size_t c) const { return mHeader[c]; }

	inline bool isValid() const { return mRowCount > 0 && mColumnCount > 0; }

	bool write(const std::filesystem::path& path, char separator = ';') const;
	bool write(std::ostream& stream, char separator = ';') const;

private:
	size_t mRowCount;
	size_t mColumnCount;

	std::vector<std::string> mHeader;
	std::vector<float> mData;
};

} // namespace PR
