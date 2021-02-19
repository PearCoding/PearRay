#include "CSV.h"
#include "Logger.h"

#include <cctype>
#include <fstream>

namespace PR {

CSV::CSV(size_t rows, size_t cols)
	: mRowCount(rows)
	, mColumnCount(cols)
	, mHeader(cols)
	, mData(rows * cols, 0.0f)
{
}

static inline bool isSeparator(int c)
{
	return c == ',' || c == ';';
}

CSV::CSV(const std::filesystem::path& path)
	: mRowCount(0)
	, mColumnCount(0)
{
	read(path);
}

CSV::CSV(std::istream& stream)
	: mRowCount(0)
	, mColumnCount(0)
{
	read(stream);
}

bool CSV::read(const std::filesystem::path& path)
{
	std::fstream stream;
	stream.open(path.c_str(), std::ios::in);
	if (!stream)
		return false;

	return read(stream);
}

bool CSV::read(std::istream& stream)
{
	if (!stream)
		return false;

	mData.clear();
	mHeader.clear();

	// Get first row and detect seperator
	std::vector<std::string> first_row;

	std::string line;
	std::getline(stream, line);

	std::string token;
	for (const auto& c : line) {
		if (isSeparator(c)) {
			if (!token.empty())
				first_row.push_back(token);
			token.clear();
		} else {
			token += c;
		}
	}
	if (!token.empty())
		first_row.push_back(token);

	mColumnCount = first_row.size();
	if (mColumnCount == 0)
		return false;

	// Make sure the locale is correct
	const char* oldLocale = std::setlocale(LC_ALL, "C");

	// Check if first row is the header...
	// There is no standard way, but we just assume the first row has strings, the data does not
	bool hasHeader = false;
	for (const auto& value : first_row) {
		try {
			std::stof(value);
		} catch (...) {
			hasHeader = true;
			break;
		}
	}

	// Use first_row as header or data
	if (hasHeader) {
		mHeader = first_row;
	} else {
		mHeader.resize(mColumnCount, ""); // Set to empty
		mData.reserve(mColumnCount);
		for (const auto& value : first_row)
			mData.push_back(std::stof(value));
	}

	// Get other parts
	bool hadInvalidLine		  = false;
	bool hadInvalidConvertion = false;

	// TODO: Maybe support comments?
	while (std::getline(stream, line)) {
		// Get tokens of the current line
		size_t counter = 0;
		token.clear();
		for (const auto& c : line) {
			if (isSeparator(c)) {
				if (!token.empty())
					first_row[counter++] = token;
				token.clear();
			} else {
				token += c;
			}

			if (counter == mColumnCount)
				break;
		}

		if (!token.empty() && counter != mColumnCount)
			first_row[counter++] = token;

		if (counter != mColumnCount) {
			hadInvalidLine = true;
			continue;
		}

		// Convert tokens to floating point numbers
		for (const auto& value : first_row) {
			try {
				size_t pos = 0;
				mData.push_back(std::stof(value, &pos));
				if (pos != value.size())
					hadInvalidConvertion = true;
			} catch (const std::invalid_argument&) {
				hadInvalidConvertion = true;
				mData.push_back(0.0f);
			} catch (const std::out_of_range&) {
				// Silently accept
				mData.push_back(0.0f);
			}
		}
	}

	if (hadInvalidConvertion)
		PR_LOG(L_ERROR) << "CSV has invalid entries. Invalid entries are set to zero" << std::endl;

	if (hadInvalidLine)
		PR_LOG(L_ERROR) << "CSV has malformed lines. Lines with invalid entries are skipped" << std::endl;

	if (mData.size() % mColumnCount != 0) // Should never trigger by design, but better safe than sorry
		PR_LOG(L_ERROR) << "CSV has no equal sized lines" << std::endl;

	mRowCount = mData.size() / mColumnCount;
	mData.shrink_to_fit();

	// Set locale back to the original
	std::setlocale(LC_ALL, oldLocale);

	return true;
}

bool CSV::write(const std::filesystem::path& path, char separator) const
{
	if (!isValid())
		return false;

	std::fstream stream;
	stream.open(path.c_str(), std::ios::out);
	if (!stream)
		return false;

	return write(stream, separator);
}

bool CSV::write(std::ostream& stream, char separator) const
{
	// Check if we have a (useable) header
	bool hasHeader = false;
	for (const auto& str : mHeader) {
		if (!str.empty()) {
			hasHeader = true;
			break;
		}
	}

	// Write out header if available
	if (hasHeader) {
		for (auto it = mHeader.begin(); it != mHeader.end(); ++it) {
			stream << *it;
			if ((it + 1) != mHeader.end())
				stream << separator;
		}
		stream << std::endl;
	}

	// Write out data
	for (size_t i = 0; i < mRowCount; ++i) {
		for (size_t j = 0; j < mColumnCount; ++j) {
			stream << (*this)(i, j);
			if (j != mColumnCount - 1)
				stream << separator;
		}
		stream << std::endl;
	}

	return true;
}

} // namespace PR
