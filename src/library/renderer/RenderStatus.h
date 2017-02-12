#pragma once

#include "PR_Config.h"

#include <unordered_set>
#include <unordered_map>

namespace PR
{
	class PR_LIB RenderStatus
	{
	public:
		RenderStatus();

		void setPercentage(float f);
		float percentage() const;

		void setField(const std::string& unique_name,
			const std::string& short_name,
			const std::string& grp, double f);
		double getField(const std::string& unique_name) const;

		std::unordered_set<std::string> getGroups() const;

		template<typename F>
		void for_each(F f) const;

		template<typename F>
		void for_each_group(const std::string& grp, F f) const;

	private:
		struct Field
		{
			std::string ShortName;
			std::string GroupName;
			double Value;
		};

		float mPercentage;
		std::unordered_map<std::string, Field> mFields;
	};

	template<typename F>
	void RenderStatus::for_each(F f) const
	{
		for(const auto& elem : mFields)
			f(elem.first, elem.second.ShortName, elem.second.GroupName, elem.second.Value);
	}

	template<typename F>
	void RenderStatus::for_each_group(const std::string& grp, F f) const
	{
		for(const auto& elem : mFields)
		{
			if(elem.second.GroupName == grp)
				f(elem.first, elem.second.ShortName, elem.second.GroupName, elem.second.Value);
		}
	}
}
