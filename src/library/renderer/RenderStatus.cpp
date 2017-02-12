#include "RenderStatus.h"

namespace PR
{
	RenderStatus::RenderStatus() : mPercentage(0)
	{
	}

	void RenderStatus::setPercentage(float f)
	{
		mPercentage = f;
	}

	float RenderStatus::percentage() const
	{
		return mPercentage;
	}

	void RenderStatus::setField(const std::string& unique_name,
		const std::string& short_name,
		const std::string& grp, double v)
	{
		Field f;
		f.ShortName = short_name;
		f.GroupName = grp;
		f.Value = v;

		mFields[unique_name] = f;
	}

	double RenderStatus::getField(const std::string& unique_name) const
	{
		return mFields.at(unique_name).Value;
	}

	std::unordered_set<std::string> RenderStatus::getGroups() const
	{
		std::unordered_set<std::string> set;
		for(const auto& elem : mFields)
			set.insert(elem.second.GroupName);

		return set;
	}
}
