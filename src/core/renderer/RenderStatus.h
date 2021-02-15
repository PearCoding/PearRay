#pragma once

#include "PR_Config.h"

#include <unordered_map>
#include <variant>

namespace PR {
class PR_LIB_CORE RenderStatus {
public:
	using Field = std::variant<float, uint64, std::string>;
	typedef std::unordered_map<std::string, Field> map_t;

	RenderStatus();

	void setPercentage(double f);
	double percentage() const;

	void setField(const std::string& unique_name, const Field& f);
	bool hasField(const std::string& unique_name) const;
	Field getField(const std::string& unique_name) const;

	inline size_t fieldCount() const { return mFields.size(); }

	map_t::const_iterator begin() const;
	map_t::const_iterator end() const;

private:
	double mPercentage;
	std::unordered_map<std::string, Field> mFields;
};
} // namespace PR
