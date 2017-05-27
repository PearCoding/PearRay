#pragma once

#include "Variant.h"

#include <unordered_map>

namespace PR {
class PR_LIB RenderStatus {
public:
	typedef std::unordered_map<std::string, Variant> map_t;

	RenderStatus();

	void setPercentage(float f);
	float percentage() const;

	void setField(const std::string& unique_name, const Variant& f);
	bool hasField(const std::string& unique_name) const;
	const Variant& getField(const std::string& unique_name) const;

	map_t::const_iterator begin() const;
	map_t::const_iterator end() const;

private:
	float mPercentage;
	std::unordered_map<std::string, Variant> mFields;
};
}
