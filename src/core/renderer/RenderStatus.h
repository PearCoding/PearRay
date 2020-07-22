#pragma once

#include "Variant.h"

#include <unordered_map>

namespace PR {
class PR_LIB_CORE RenderStatus {
public:
	typedef std::unordered_map<std::string, Variant> map_t;

	RenderStatus();

	void setPercentage(double f);
	double percentage() const;

	void setField(const std::string& unique_name, const Variant& f);
	bool hasField(const std::string& unique_name) const;
	Variant getField(const std::string& unique_name) const;

	map_t::const_iterator begin() const;
	map_t::const_iterator end() const;

private:
	double mPercentage;
	std::unordered_map<std::string, Variant> mFields;
};
}
